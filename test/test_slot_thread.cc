#include "catch.hpp"

#include "wink/thread/slot_thread.hpp"

using namespace std;
using namespace wink;

TEST_CASE( "slot_thread can prepare slot automatic", "[thread]" ) {

    SECTION( "thread work fine" ) {
        int count = 0;
        slot_thread back ([&] (int inc) {
            count += inc;
        }, 3);
        back.join();

        REQUIRE(count == 3);
    }

    SECTION( "slot_hub is prepared" ) {

        std::shared_ptr<slot_hub> thread_slot;

        // NOTE: Catch seems not complete support the cross-thread SECTION
        // This test not stable with the thread (may cause crash).
        slot_thread back ([&] {
            thread_slot = slot_hub::my_hub();
        });

        SECTION( "slot_hub on other thread" ) {
            REQUIRE_FALSE( slot_hub::loop() );
            REQUIRE_FALSE( slot_hub::my_hub() );
            REQUIRE_FALSE( thread_slot->same_thread() );
        }

        back.join();
    }

}
