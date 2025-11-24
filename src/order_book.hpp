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
    using Orders = unordered_map<uint64_t, PriceAndSide>;
    using SideLevels = unordered_map<uint64_t, LevelOrders>;
    Orders orders_by_id;
    SideLevels bid_orders;
    SideLevels ask_orders;
    void update_book(const MboMsg &msg);
    void clear_book();
    void add_order(MboMsg &mbo);
    SideLevels& get_side_levels(Side side);
};

#endif //ORDER_BOOK