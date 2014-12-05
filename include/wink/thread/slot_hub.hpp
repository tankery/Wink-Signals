///
/// Wink Signals
/// Copyright (C) 2013-2014 Tankery Chen (tankery.chen@gmail.com)
///
///
/// This software is provided 'as-is', without any express or implied warranty.
/// In no event will the authors be held liable for any damages arising from the
/// use of this software.
///
/// Permission is hereby granted, free of charge, to any person
/// obtaining a copy of this software and associated documentation files (the "Software"),
/// to deal in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// 1. The origin of this software must not be misrepresented;
///    you must not claim that you wrote the original software.
///    If you use this software in a product, an acknowledgment
///    in the product documentation would be appreciated but is not required.
///
/// 2. Altered source versions must be plainly marked as such,
///    and must not be misrepresented as being the original software.
///
/// 3. The above copyright notice and this permission notice shall be included in
///    all copies or substantial portions of the Software.
///

#ifndef WINK_SLOT_LOOP_HPP
#define WINK_SLOT_LOOP_HPP

#include <cassert>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#ifdef __APPLE__
#include <mutex>
#include <unordered_map>
#endif
#include "wink/detail/FastDelegate.h"
#include "wink/thread/utility.hpp"
#include "wink/thread/queue.hpp"

namespace wink
{
    /// \brief Describes a slot hub that work on specific thread.
    ///
    /// The hub can store up comming slot in a queue, and invoke it on the thread it link to.
    ///
    /// \author Tankery Chen
    class slot_hub : public noncopyable
    {
    private:
        typedef std::thread::id tid_t;
        typedef std::shared_ptr<slot_hub> sptr_this_t;
        typedef std::function<void()> func_t;
        typedef std::unique_ptr<func_t> uptr_func_t;
        typedef wink::queue<func_t> queue_t;

#ifdef __APPLE__
        // Apple not support the thread_local, so I do some tricky here.
        // TODO: destroy slot_hub when thread exit.
        typedef std::unordered_map<tid_t, sptr_this_t> hub_table_t;
        static hub_table_t s_hub_table;
        static std::mutex m_table_mutex;
#else
        thread_local static sptr_this_t s_instance;
#endif

        queue_t m_queue;
        const tid_t m_tid;
        std::atomic<bool> m_alive;

        slot_hub();

    public:

        static bool prepare();

        // explicit destroy the instance.
        static void destroy();

        static bool loop(bool blocking = false);

        static inline sptr_this_t my_hub()
        {
#ifdef __APPLE__
            auto this_tid = std::this_thread::get_id();
            std::unique_lock<std::mutex> lock(m_table_mutex);
            return s_hub_table[this_tid];
#else
            return s_instance;
#endif
        }

        template<typename R, typename... Args>
        bool send(fastdelegate::FastDelegate< R(Args...)>&& delegate, Args&&... args)
        {
            return send(delegate, std::forward<Args>(args)...);
        }

        template<typename R, typename... Args>
        bool send(const fastdelegate::FastDelegate< R(Args...)>& delegate, Args&&... args)
        {
            if (!is_alive())
                return false;
            auto func = std::bind(delegate, std::forward<Args>(args)...);
            queue().push(std::move([=] { func(); }));

            return true;
        }

        void stop(bool clearSlots = true);

        inline bool is_alive()
        {
            return m_alive;
        }

        inline bool is_idel()
        {
            return m_queue.empty();
        }

        inline queue_t& queue()
        {
            return m_queue;
        }

        inline bool same_thread() const
        {
            auto this_tid = std::this_thread::get_id();
            return this_tid == m_tid;
        }
    };

}


#endif // WINK_SLOT_LOOP_HPP
