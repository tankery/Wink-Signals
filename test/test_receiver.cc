#include "catch.hpp"

#include <thread>
#include "wink/thread/receiver.hpp"

using namespace std;
using namespace wink;


TEST_CASE( "receiver constructor test", "[thread]" ) {

    SECTION( "construct without prepare" ) {
        REQUIRE_FALSE(receiver().slothub.lock());
    }

    SECTION( "construct with prepare" ) {
        slot_hub::prepare();
        REQUIRE(receiver().slothub.lock());

        slot_hub::destroy();
        REQUIRE_FALSE(receiver().slothub.lock());
    }

    SECTION( "receiver's hub is weak" ) {
        slot_hub::prepare();
        receiver rc;

        slot_hub::destroy();
        REQUIRE_FALSE(rc.slothub.lock());
    }

    SECTION( "construct with other thread's hub" ) {

        std::shared_ptr<slot_hub> thread_slot;
        std::mutex m;
        std::condition_variable thread_ready;
        std::condition_variable check_finished;

        // NOTE: Catch seems not complete support the cross-thread SECTION
        // This test not stable with the thread (may cause crash).
        thread back ([&] {
            slot_hub::prepare();
            thread_slot = slot_hub::my_hub();
            thread_ready.notify_one();

            {
                std::unique_lock<std::mutex> lock(m);
                check_finished.wait(lock, [&] { return !thread_slot; });
            }

            slot_hub::destroy();
        });

        {
            std::unique_lock<std::mutex> lock(m);
            thread_ready.wait(lock, [&] { return thread_slot; });
        }

        receiver rc(thread_slot);
        {
            std::shared_ptr<slot_hub> rc_slot = rc.slothub.lock();
            REQUIRE(rc_slot);
            REQUIRE_FALSE(rc_slot->same_thread());
        }

        thread_slot = nullptr;
        check_finished.notify_one();
        back.join();

        REQUIRE_FALSE(rc.slothub.lock());

    }

}
