///
/// Wink Signals
/// Copyright (C) 2013-2014 Miguel Martin (miguel@miguel-martin.com)
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

#ifndef WINK_RECEIVER_HPP
#define WINK_RECEIVER_HPP

#include "wink/thread/slot_hub.hpp"

namespace wink
{
    struct receiver {

        std::weak_ptr<slot_hub> slothub;

        receiver()
            : slothub(slot_hub::my_hub())
        {}

        receiver(std::shared_ptr<slot_hub> hub)
            : slothub(hub)
        {}

        // Receiver transform the slot emit event. User can Override this method
        // to do something on signal thread when emit.
        //
        // This will be useful when you want to connet slot on platform UI thread,
        // which may not possiable to run the blocking slot loop.
        // User can send a event to UI thread, so it can invoke a non-blocking loop
        // when signal emit.
        //
        // For example, on Android, the UI thread can't be blocked. So we override
        // the emit, send a DO_LOOP message to UI thread after send the slot. When
        // UI thread handling this message, it can invoke a loop (on UI thread) to
        // call the slots in queue.
        template<typename R, typename... Args>
        bool emit(const fastdelegate::FastDelegate< R(Args...)>& delegate, Args&&... args)
        {
            std::shared_ptr<slot_hub> hub = slothub.lock();
            if (hub && !hub->same_thread())
                return hub->send(delegate, std::forward<Args>(args)...);
            else
                delegate(std::forward<Args>(args)...);

            return false;
        }

    };
}


#endif // WINK_RECEIVER_HPP
