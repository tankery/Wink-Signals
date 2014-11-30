#include "catch.hpp"

#include <thread>
#include "wink/thread/slot_hub.hpp"

using namespace std;
using namespace wink;
using namespace fastdelegate;

struct Receiver {
    int count;

    Receiver() : count(0) {}

    void increase(int i) { count+= i; }
};

TEST_CASE( "slot_hub can't use without prepare", "[queue]" ) {

    SECTION( "Use slot_hub without prepare" ) {
        REQUIRE_FALSE( slot_hub::loop() );
    }

    SECTION( "Use slot_hub with prepare" ) {
        REQUIRE( slot_hub::prepare() );

        SECTION( "Use slot_hub basic functions" ) {
            REQUIRE( slot_hub::loop() );
            REQUIRE( slot_hub::my_hub() );
            REQUIRE( slot_hub::my_hub()->same_thread() );
        }

        SECTION( "Construct delegate, send it, then loop" ) {
            Receiver rc;
            FastDelegate<void(int)> delegate(&rc, &Receiver::increase);

            REQUIRE( slot_hub::my_hub()->is_alive() );

            REQUIRE( slot_hub::my_hub()->send(std::move(delegate), 3) );
            REQUIRE( rc.count == 0 );

            REQUIRE( slot_hub::loop() );
            REQUIRE( rc.count == 3 );

            slot_hub::my_hub()->stop();
            // stop will triggled after next loop call.
            REQUIRE( slot_hub::my_hub()->is_alive() );
            REQUIRE( slot_hub::loop() );
            REQUIRE_FALSE( slot_hub::my_hub()->is_alive() );
        }

        slot_hub::destroy();

    }

    SECTION( "Use slot_hub with thread" ) {

        std::shared_ptr<slot_hub> thread_slot;
        std::mutex m;
        std::condition_variable thread_ready;

        // NOTE: Catch seems not complete support the cross-thread SECTION
        // This test not stable with the thread (may cause crash).
        thread back ([&] {
            REQUIRE( slot_hub::prepare() );
            thread_slot = slot_hub::my_hub();
            thread_ready.notify_one();

            SECTION( "Use slot_hub basic functions in thread" ) {
                REQUIRE( slot_hub::loop() );
                REQUIRE( thread_slot );
                REQUIRE( thread_slot->same_thread() );
            }

            while (thread_slot->is_alive()) {
                slot_hub::loop();
            }

            slot_hub::destroy();
        });

        {
            std::unique_lock<std::mutex> lock(m);
            thread_ready.wait(lock, [&] { return thread_slot; });
        }

        SECTION( "Use slot_hub basic functions out of thread" ) {
            REQUIRE_FALSE( slot_hub::loop() );
            REQUIRE_FALSE( slot_hub::my_hub() );
            REQUIRE_FALSE( thread_slot->same_thread() );
        }

        SECTION( "Construct delegate, send it, then loop" ) {
            Receiver rc;
            FastDelegate<void(int)> delegate(&rc, &Receiver::increase);

            REQUIRE( rc.count == 0 );
            REQUIRE( thread_slot->send(std::move(delegate), 3) );
            while (!thread_slot->is_idel())
                ;
            REQUIRE( rc.count == 3 );
        }

        thread_slot->stop();
        back.join();

    }

}
