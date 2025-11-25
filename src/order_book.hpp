#ifndef ORDER_BOOK
#define ORDER_BOOK

#include <cstdint>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

struct PriceAndSide{
    int64_t price;
    Side side;
};

class OrderBook{
public:
    using LevelOrders = vector<MboMsg>;
    using Orders = unordered_map<uint64_t, PriceAndSide>; //map from order id to price/side
    using SideLevels = unordered_map<uint64_t, LevelOrders>; //map from price to orders at that price
    Orders orders_by_id;
    SideLevels bid_orders;
    SideLevels ask_orders;
    void update_book(MboMsg &msg);
    void clear_book();
    void add_order(MboMsg &msg);
    void cancel_order(MboMsg &msg);
    void modify_order(MboMsg &msg);
    SideLevels& get_side_levels(Side side);
};

#endif //ORDER_BOOK