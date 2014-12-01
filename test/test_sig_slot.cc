#include "catch.hpp"

#include "wink/signal.hpp"
#include "wink/thread/slot_thread.hpp"

using namespace std;
using namespace wink;

struct Handler {

    std::atomic<int> counter;

    Handler() : counter(0) {}

    /// Handles an event that sends an integer
    void handleEvent(int inc)
    {
        counter += inc;
    }
};

struct Receiver : receiver {

    std::atomic<int> counter;

    std::mutex tid_m;
    std::condition_variable tid_set;
    bool is_set;
    std::thread::id tid;

    Receiver() : counter(0), is_set(false) {}

    /// Handles an event that sends an integer
    void handleEvent(int inc)
    {
        counter += inc;
        {
            std::unique_lock<std::mutex> lock(tid_m);
            tid = std::this_thread::get_id();
            is_set = true;
            tid_set.notify_one();
        }
    }

};

TEST_CASE( "normal sig-slot can work", "[thread]" ) {


    typedef signal<slot<void (int)> > sig_t;
    Handler hd;
    sig_t sender;
    
    sender.connect(&hd, &Handler::handleEvent);

    sender(3);

    REQUIRE(hd.counter == 3);

}

TEST_CASE( "slot in thread can work", "[thread]" ) {

    typedef signal<slot<void (int)> > sig_t;

    SECTION( "slot in thread same as signal" ) {
        slot_hub::prepare();

        Receiver rc;
        sig_t sender;
        sender.connect(&rc, &Receiver::handleEvent);

        sender(3);
        REQUIRE(rc.counter == 3);

        slot_hub::destroy();
    }

    SECTION( "slot in thread different as signal" ) {
        std::shared_ptr<Receiver> prc;

        slot_thread back([&] {
            prc = std::make_shared<Receiver>();
        });

        sig_t sender;
        sender.connect(prc.get(), &Receiver::handleEvent);

        sender(3);

        {
            std::unique_lock<std::mutex> lock(prc->tid_m);
            prc->tid_set.wait(lock, [&] { return prc->is_set; });
        }

        REQUIRE(prc->counter == 3);
        REQUIRE(prc->tid != std::this_thread::get_id());
        REQUIRE(prc->tid == back.get_id());

        back.join();
    }

}
