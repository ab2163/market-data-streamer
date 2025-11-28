#include "../src/server.hpp"
#include "../src/book_constructor.hpp"
#include "../src/config.hpp"

#include <thread>
#include <chrono>
#include <memory>

#include <catch2/catch_test_macros.hpp>

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
    m.flags = {};
    return m;
}

TEST_CASE("Multiple clients receive identical broadcast and reconstruct identical books"){
    std::unique_ptr<BookConstructor> constructor1;
    std::unique_ptr<BookConstructor> constructor2;
    std::thread client_thread_1;
    std::thread client_thread_2;

    {
        Server server;

        std::thread server_thread([&](){
            server.start_listening();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        constructor1 = std::make_unique<BookConstructor>();
        constructor2 = std::make_unique<BookConstructor>();

        std::thread t1([&](){
            constructor1->build_order_book();
        });

        std::thread t2([&](){
            constructor2->build_order_book();
        });

        client_thread_1.swap(t1);
        client_thread_2.swap(t2);

        //give server time to accept both client connections
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::vector<MboMsg> msgs;
        msgs.push_back(make_mbo(1, 10000, 10, Side::Bid, Action::Add));
        msgs.push_back(make_mbo(2,  9000,  8, Side::Bid, Action::Add));
        msgs.push_back(make_mbo(3, 11000,  6, Side::Ask, Action::Add));

        //id 2: move from 9000 -> 12000
        msgs.push_back(make_mbo(2, 12000, 8, Side::Bid, Action::Modify));

        //id 1: partial cancel 4 of 10
        msgs.push_back(make_mbo(1, 0, 4, Side::Bid, Action::Cancel));

        server.broadcast(msgs, true);
        server.wait_to_finish();
        server_thread.join();
    }

    client_thread_1.join();
    client_thread_2.join();

    OrderBook &book1 = constructor1->order_book;
    OrderBook &book2 = constructor2->order_book;

    PriceLevel bid1 = book1.get_bid_level();
    PriceLevel ask1 = book1.get_ask_level();
    PriceLevel bid2 = book2.get_bid_level();
    PriceLevel ask2 = book2.get_ask_level();

    REQUIRE(bid1.price == 12000);
    REQUIRE(bid1.size == 8);
    REQUIRE(bid1.count == 1);

    REQUIRE(ask1.price == 11000);
    REQUIRE(ask1.size == 6);
    REQUIRE(ask1.count == 1);

    REQUIRE(bid2.price == bid1.price);
    REQUIRE(bid2.size == bid1.size);
    REQUIRE(bid2.count == bid1.count);

    REQUIRE(ask2.price == ask1.price);
    REQUIRE(ask2.size == ask1.size);
    REQUIRE(ask2.count == ask1.count);
}
