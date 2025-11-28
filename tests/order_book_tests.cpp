#include <catch2/catch_test_macros.hpp>
#include "../src/order_book.hpp"

//helper to construct an MboMsg with sane defaults.
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
    m.flags = {}; //no TOB flag, plain limit order
    return m;
}

TEST_CASE("update_book ignores messages with Side::None and increments error counter"){
    OrderBook ob;

    auto msg = make_mbo(1, 10000, 10, Side::None, Action::Add);
    ob.update_book(msg);

    REQUIRE(ob.orders_by_id.empty());
    REQUIRE(ob.bid_orders.empty());
    REQUIRE(ob.ask_orders.empty());
    REQUIRE(ob.best_bid_px == kUndefPrice);
    REQUIRE(ob.best_ask_px == kUndefPrice);
    REQUIRE(ob.msgs_error == 1);
}

TEST_CASE("add_order for a bid populates structures and updates best_bid_px"){
    OrderBook ob;

    auto msg = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg);

    //order is tracked by id
    REQUIRE(ob.orders_by_id.size() == 1);
    REQUIRE(ob.orders_by_id.count(1) == 1);

    //level exists on bid side
    REQUIRE(ob.bid_orders.size() == 1);
    auto level_it = ob.bid_orders.find(10000);
    REQUIRE(level_it != ob.bid_orders.end());
    REQUIRE(level_it->second.size() == 1);
    REQUIRE(level_it->second.front().order_id == 1);

    //best bid price updated
    REQUIRE(ob.best_bid_px == 10000);

    //get_bid_level aggregates correctly
    PriceLevel pl = ob.get_bid_level();
    REQUIRE(pl.price == 10000);
    REQUIRE(pl.size == 10);
    REQUIRE(pl.count == 1);
}

TEST_CASE("Multiple orders at same price accumulate size and count"){
    OrderBook ob;

    auto msg1 = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg1);
    auto msg2 = make_mbo(2, 10000, 20, Side::Bid, Action::Add);
    ob.update_book(msg2);

    PriceLevel pl = ob.get_bid_level();
    REQUIRE(pl.price == 10000);
    REQUIRE(pl.size == 30);   //10 + 20
    REQUIRE(pl.count == 2);   //two orders at that level
}

TEST_CASE("Best bid and ask are chosen as max bid and min ask"){
    OrderBook ob;

    //bids at 100, 105, 103
    auto msg1 = make_mbo(1, 10000, 5, Side::Bid, Action::Add);
    ob.update_book(msg1);
    auto msg2 = make_mbo(2, 10500, 5, Side::Bid, Action::Add);
    ob.update_book(msg2);
    auto msg3 = make_mbo(3, 10300, 5, Side::Bid, Action::Add);
    ob.update_book(msg3);

    //asks at 110, 108, 115
    auto msg4 = make_mbo(4, 11000, 5, Side::Ask, Action::Add);
    ob.update_book(msg4);
    auto msg5 = make_mbo(5, 10800, 5, Side::Ask, Action::Add);
    ob.update_book(msg5);
    auto msg6 = make_mbo(6, 11500, 5, Side::Ask, Action::Add);
    ob.update_book(msg6);

    PriceLevel bid = ob.get_bid_level();
    PriceLevel ask = ob.get_ask_level();

    REQUIRE(bid.price == 10500); //highest bid
    REQUIRE(ask.price == 10800); //lowest ask
}

TEST_CASE("Full cancel removes order, level, and recomputes best price when needed"){
    OrderBook ob;

    //two bid levels: 10000 and 9000
    auto msg1 = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg1);
    auto msg2 = make_mbo(2,  9000, 10, Side::Bid, Action::Add);
    ob.update_book(msg2);

    REQUIRE(ob.get_bid_level().price == 10000);

    //cancel the top-of-book order fully
    auto cancel_msg = make_mbo(1, 0, 10, Side::Bid, Action::Cancel);
    ob.update_book(cancel_msg);

    //order removed from id map
    REQUIRE(ob.orders_by_id.count(1) == 0);

    //best should move to 9000
    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 9000);
    REQUIRE(bid.size == 10);
    REQUIRE(bid.count == 1);
}

TEST_CASE("Partial cancel only reduces size and keeps level"){
    OrderBook ob;

    auto msg = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg);

    //cancel 4 of 10
    auto cancel_msg = make_mbo(1, 0, 4, Side::Bid, Action::Cancel);
    ob.update_book(cancel_msg);

    //order still exists
    REQUIRE(ob.orders_by_id.count(1) == 1);

    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 10000);
    REQUIRE(bid.size == 6);   //10 - 4
    REQUIRE(bid.count == 1);
}

TEST_CASE("Cancelling unknown order id increments msgs_error and leaves book unchanged"){
    OrderBook ob;

    auto msg = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg);
    REQUIRE(ob.msgs_error == 0);

    //cancel an id that does not exist
    auto cancel_msg = make_mbo(999, 0, 10, Side::Bid, Action::Cancel);
    ob.update_book(cancel_msg);

    REQUIRE(ob.msgs_error == 1);

    //book content unchanged
    REQUIRE(ob.orders_by_id.size() == 1);
    REQUIRE(ob.orders_by_id.count(1) == 1);
    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 10000);
    REQUIRE(bid.size == 10);
    REQUIRE(bid.count == 1);
}

TEST_CASE("Modify with same price and smaller size keeps priority and just shrinks size"){
    OrderBook ob;

    auto msg = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg);

    //reduce size from 10 to 6 at same price
    auto mod = make_mbo(1, 10000, 6, Side::Bid, Action::Modify);
    ob.update_book(mod);

    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 10000);
    REQUIRE(bid.size == 6);
    REQUIRE(bid.count == 1);
}

TEST_CASE("Modify with price change moves order between levels and updates best price"){
    OrderBook ob;

    //two bid levels
    auto msg1 = make_mbo(1, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg1);
    auto msg2 = make_mbo(2,  9000, 10, Side::Bid, Action::Add);
    ob.update_book(msg2);

    REQUIRE(ob.get_bid_level().price == 10000);

    //move order 2 from 9000 -> 11000
    auto mod = make_mbo(2, 11000, 10, Side::Bid, Action::Modify);
    ob.update_book(mod);

    //old level at 9000 should now be empty, and 11000 should be best
    REQUIRE(ob.bid_orders.count(9000) == 0);
    REQUIRE(ob.bid_orders.count(11000) == 1);

    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 11000);
    REQUIRE(bid.size == 10);
    REQUIRE(bid.count == 1);
}

TEST_CASE("Modify on unknown id is treated as fresh add"){
    OrderBook ob;

    REQUIRE(ob.orders_by_id.empty());

    auto mod = make_mbo(42, 10000, 10, Side::Bid, Action::Modify);
    ob.update_book(mod);

    //should behave like an add
    REQUIRE(ob.orders_by_id.size() == 1);
    REQUIRE(ob.orders_by_id.count(42) == 1);
    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 10000);
    REQUIRE(bid.size == 10);
    REQUIRE(bid.count == 1);
}

TEST_CASE("get_bid_level recomputes stale best_bid_px when map changes"){
    OrderBook ob;

    //manually create inconsistency: best_bid_px points to a missing level
    auto msg1 = make_mbo(1,  9000, 10, Side::Bid, Action::Add);
    ob.update_book(msg1);
    auto msg2 = make_mbo(2, 10000, 10, Side::Bid, Action::Add);
    ob.update_book(msg2);

    REQUIRE(ob.best_bid_px == 10000);

    //remove level at 10000 directly to simulate stale state
    auto it = ob.bid_orders.find(10000);
    REQUIRE(it != ob.bid_orders.end());
    ob.bid_orders.erase(it);

    //best_bid_px still 10000 but there is only 9000 in the map
    REQUIRE(ob.best_bid_px == 10000);
    REQUIRE(ob.bid_orders.count(9000) == 1);

    //get_bid_level should detect stale price and recompute
    PriceLevel bid = ob.get_bid_level();
    REQUIRE(bid.price == 9000);
    REQUIRE(bid.size == 10);
    REQUIRE(bid.count == 1);
}
