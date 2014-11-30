#include "catch.hpp"

#include <thread>
#include "wink/queue.hpp"

using namespace std;
using namespace wink;


TEST_CASE( "Wink queue is a blocking queue", "[queue]" ) {

    queue<int> q;

    SECTION( "push and pop are correct in same thread." ) {
        for (int i = 0; i < 10; i++)
            q.push(i);

        REQUIRE(q.size() == 10);

        for (int i = 0; i < 10; i++) {
            int v = q.pop();
            REQUIRE(v == i);
        }

        REQUIRE(q.size() == 0);
    }

    SECTION( "push and pop are correct in different thread." ) {

        thread back ([&] {
            for (int i = 0; i < 10; i++)
                q.push(i);
        });

        for (int i = 0; i < 10; i++) {
            int v = q.pop();
            REQUIRE(v == i);
        }

        REQUIRE(q.size() == 0);

        back.join();
    }

}
