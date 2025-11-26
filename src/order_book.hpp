#ifndef ORDER_BOOK
#define ORDER_BOOK

#include <cstdint>

#include <databento/historical.hpp>
#include <databento/pretty.hpp>

using namespace std;
using namespace databento;

struct PriceAndSide{
    int64_t price;
    Side side;
};

struct PriceLevel{
    int64_t price{kUndefPrice};
    uint32_t size{0};
    uint32_t count{0};

    bool IsEmpty() const { return price == kUndefPrice; }
    operator bool() const { return !IsEmpty(); }
};

class OrderBook{
public:
    using LevelOrders = vector<MboMsg>;
    using Orders = unordered_map<uint64_t, PriceAndSide>; //map from order id to price/side
    using SideLevels = unordered_map<int64_t, LevelOrders>; //map from price to orders at that price
    int64_t best_bid_px{kUndefPrice};
    int64_t best_ask_px{kUndefPrice};
    Orders orders_by_id;
    SideLevels bid_orders;
    SideLevels ask_orders;
    void update_book(MboMsg &msg);
    void clear_book();
    void add_order(MboMsg &msg);
    void cancel_order(MboMsg &msg);
    void modify_order(MboMsg &msg);
    SideLevels& get_side_levels(Side side);
    PriceLevel get_price_level(int64_t price, const LevelOrders &level);
    PriceLevel get_bid_level();
    PriceLevel get_ask_level();
    void print_BBO(MboMsg &msg);
    void recompute_px(Side side, int64_t removed_price);
    void recompute_best_bid();
    void recompute_best_ask();
};

#endif //ORDER_BOOK