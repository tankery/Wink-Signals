#include "catch.hpp"

#include <functional>
#include <thread>
#include "wink/thread/queue.hpp"

using namespace std;
using namespace wink;


TEST_CASE( "Wink queue is a blocking queue", "[queue]" ) {

    queue<int> q;

    SECTION( "push and pop are correct in same thread." ) {
        for (int i = 0; i < 10; i++)
            q.push(i);

        REQUIRE(q.size() == 10);

        for (int i = 0; i < 10; i++) {
            auto pv = q.pop();
            REQUIRE(pv);
            REQUIRE(*pv == i);
        }

        REQUIRE(q.size() == 0);
    }

    SECTION( "clear are correct in same thread." ) {
        for (int i = 0; i < 10; i++)
            q.push(i);

        REQUIRE(q.size() == 10);

        q.clear();

        REQUIRE(q.size() == 0);
    }

    SECTION( "push and pop are correct in different thread." ) {

        thread back ([&] {
            for (int i = 0; i < 10; i++)
                q.push(i);
        });

        for (int i = 0; i < 10; i++) {
            auto pv = q.pop();
            REQUIRE(pv);
            REQUIRE(*pv == i);
        }

        REQUIRE(q.size() == 0);

        back.join();
    }

    SECTION( "push and try_pop are correct in same thread." ) {
        for (int i = 0; i < 10; i++)
            q.push(i);

        REQUIRE(q.size() == 10);

        for (int i = 0; i < 10; i++) {
            auto pv = q.try_pop();
            REQUIRE(pv);
            REQUIRE(*pv == i);
        }

        REQUIRE(q.size() == 0);
    }

}


TEST_CASE( "Wink queue can store the function also", "[queue]" ) {

    queue<function<void()>> q;

    SECTION( "push and pop are correct in same thread." ) {
        int count = 0;
        q.push([&] { count++; });

        REQUIRE(q.size() == 1);
        REQUIRE(count == 0);

        auto pf = q.try_pop();
        REQUIRE(q.size() == 0);

        REQUIRE(pf);
        (*pf)();
        REQUIRE(count == 1);

    }

}
