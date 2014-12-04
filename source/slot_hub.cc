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

#include "wink/thread/slot_hub.hpp"

namespace wink
{

#ifdef __APPLE__
    slot_hub::hub_table_t slot_hub::s_hub_table;
    std::mutex slot_hub::m_table_mutex;
#else
    thread_local slot_hub::sptr_this_t slot_hub::s_instance;
#endif


slot_hub::slot_hub()
    : m_queue()
    , m_tid(std::this_thread::get_id())
    , m_alive(true)
{
}

bool slot_hub::prepare()
{
#ifdef __APPLE__
    auto this_tid = std::this_thread::get_id();

    std::unique_lock<std::mutex> lock(m_table_mutex);
    s_hub_table[this_tid] = sptr_this_t(new slot_hub);
#else
    if (s_instance)
        return false;
    s_instance = sptr_this_t(new slot_hub);
#endif

    return true;
}

void slot_hub::destroy()
{
#ifdef __APPLE__
    auto this_tid = std::this_thread::get_id();
    std::unique_lock<std::mutex> lock(m_table_mutex);
    s_hub_table[this_tid] = nullptr;
#else
    s_instance = nullptr;
#endif
}

bool slot_hub::loop(bool blocking)
{
    sptr_this_t hub = my_hub();
    if (!hub)
        return false;

    queue_t& q = hub->queue();

    uptr_func_t pfunc;
    while ( hub->m_alive && (pfunc = blocking? q.pop() : q.try_pop()) )
    {
        (*pfunc)();
    }

    return true;
}

void slot_hub::stop(bool clearSlots)
{
    if (!is_alive())
        return;
    // clear every other queue things and push a stop expression.
    if (clearSlots)
        queue().clear();
    queue().push(std::move([&] { m_alive = false; }));
}

}
