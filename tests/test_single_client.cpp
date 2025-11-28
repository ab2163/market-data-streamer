#include "../src/server.hpp"
#include "../src/book_constructor.hpp"
#include "../src/config.hpp"

#include <thread>
#include <chrono>
#include <memory>

#include <catch2/catch_test_macros.hpp>

//helper: build a simple MboMsg
static MboMsg make_mbo(
    uint64_t id,
    int64_t price,
    uint32_t size,
    Side side,
    Action action
){
    MboMsg m{};
    m.order_id = id;
    m.price = price;
    m.size = size;
    m.side = side;
    m.action = action;
    m.flags = {}; //no TOB flag
    return m;
}

TEST_CASE("Single client receives broadcast messages and reconstructs correct book end-to-end"){
    //unique pointer used for constructor so that it is created after the server
    //but does not get destroyed when the server goes out of scope
    std::unique_ptr<BookConstructor> constructor;
    std::thread client_thread;

    //server created within enclosing scope so it is destroyed before 
    //trying to join client thread
    //otherwise client thread never joins
    {
        Server server;

        //run start_listening in its own thread so it can block in accept()
        std::thread server_thread([&](){
            server.start_listening();
        });

        //give server time to bind/listen before client connects
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        //client: BookConstructor connects in its constructor
        constructor = std::make_unique<BookConstructor>();
        client_thread = std::thread([&](){
            constructor->build_order_book();
        });

        //prepare a deterministic sequence of messages
        std::vector<MboMsg> msgs;
        msgs.push_back(make_mbo(1, 10000, 10, Side::Bid, Action::Add));
        msgs.push_back(make_mbo(2, 10500,  5, Side::Bid, Action::Add));
        msgs.push_back(make_mbo(3, 11000,  7, Side::Ask, Action::Add));

        //modify order 2: price 10500 -> 10600
        msgs.push_back(make_mbo(2, 10600, 5, Side::Bid, Action::Modify));

        //cancel order 3 fully
        msgs.push_back(make_mbo(3, 0, 7, Side::Ask, Action::Cancel));

        //broadcast with last=true to close the stream
        server.broadcast(msgs, true);
        server.wait_to_finish();

        //start_listening returns after first accept and spawning accept_thread, so join now
        server_thread.join();
    }

    //wait for client to finish its receive loop
    client_thread.join();

    OrderBook &book = constructor->order_book;

    PriceLevel bid = book.get_bid_level();
    PriceLevel ask = book.get_ask_level();

    REQUIRE(bid.price == 10600);
    REQUIRE(bid.size == 5);
    REQUIRE(bid.count == 1);

    REQUIRE(ask.IsEmpty());
}
