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

#ifndef WINK_SLOT_THREAD_HPP
#define WINK_SLOT_THREAD_HPP

#include <thread>
#include "wink/thread/slot_hub.hpp"

namespace wink
{
    /// \brief Describes a thread that will prepare/destroy slot_hub before/after run.
    ///
    /// slot_thread can construct with a function and its arguments. The function and
    /// args will be called at the beginning of thread loop.
    /// The slot_thread will make sure the initialization function is finished before
    /// any other operations.
    ///
    /// \author Tankery Chen
    class slot_thread : public std::thread
    {
    private:
        std::weak_ptr<slot_hub> slothub;
        std::mutex m_initial;
        std::condition_variable thread_ready;
        bool is_ready;
    public:
        template <class Fn, class... Args>
        explicit slot_thread (Fn&& fn, Args&&... args)
            : thread([&] (Fn&& fn, Args&&... args) {
                slot_hub::prepare();
                slothub = slot_hub::my_hub();
                fn(std::forward<Args>(args)...);
                {
                    std::unique_lock<std::mutex> lock(m_initial);
                    is_ready = true;
                    thread_ready.notify_one();
                }
                slot_hub::loop(true);
                slot_hub::destroy();
            }, std::forward<Fn>(fn), std::forward<Args>(args)...)
            , is_ready(false)
        {
            // Wait for thread ready
            std::unique_lock<std::mutex> lock(m_initial);
            thread_ready.wait(lock, [&] { return is_ready; });
        }

        ~slot_thread()
        {
        }

        template <class Fn, class... Args>
        static std::unique_ptr<slot_thread> create(Fn&& fn, Args&&... args)
        {
            return std::move(std::unique_ptr<slot_thread>(new slot_thread(std::forward<Fn>(fn), std::forward<Args>(args)...)));
        }

        bool ready()
        {
            return is_ready;
        }

        void join(bool clearSlots = true) {
            std::shared_ptr<slot_hub> hub = slothub.lock();
            if (hub)
                hub->stop(clearSlots);

            std::thread::join();
        }
    };

}


#endif // WINK_SLOT_THREAD_HPP
