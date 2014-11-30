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

#ifndef WINK_QUEUE_HPP
#define WINK_QUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <deque>

namespace wink
{

    template <typename T>
    class queue
    {
    private:
        std::mutex              d_mutex;
        std::condition_variable d_condition;
        std::deque<T>           d_queue;
    public:
        inline void push(T const& value) {
            {
                std::unique_lock<std::mutex> lock(this->d_mutex);
                d_queue.push_front(value);
            }
            this->d_condition.notify_one();
        }
        inline T pop() {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
            T rc(std::move(this->d_queue.back()));
            this->d_queue.pop_back();
            return std::move(rc);
        }
        inline size_t size() {
            return this->d_queue.size();
        }
    };

}
#endif // WINK_QUEUE_HPP
