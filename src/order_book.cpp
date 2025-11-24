#include "order_book.hpp"

using namespace std;
using namespace databento;

void OrderBook::update_book(const MboMsg &msg){
    switch(msg.action){
        case Action::Clear:
            clear_book();
            break;
        case Action::Add:
            add_order(msg);
            break;
        case Action::Cancel:
            cancel_order(msg);
            break;
        case Action::Modify:
            modify_order(msg);
            break;
        case Action::Trade:
        case Action::Fill:
        case Action::None:
            break;
        default:
            throw invalid_argument{ string{"Unknown action: "} + ToString(msg.action) };
    }
}

void OrderBook::clear_book(){
    orders_by_id.clear();
    bid_orders.clear();
    ask_orders.clear();
}

SideLevels& OrderBook::get_side_levels(Side side){
    if(side == Side::Bid) return bid_orders;
    else return ask_orders;
}

void OrderBook::add_order(MboMsg &msg){
    if(msg.flags.IsTob()){
        SideLevels &levels = get_side_levels(msg.side);
        levels.clear();
        if(msg.price != kUndefPrice){
            LevelOrders level = {mbo};
            levels.emplace(mbo.price, level);
        }
    }
}

  void Add(db::MboMsg mbo) {
    if (mbo.flags.IsTob()) {
      SideLevels& levels = GetSideLevels(mbo.side);
      levels.clear();
      // kUndefPrice indicates the side's book should be cleared
      // and doesn't represent an order that should be added
      if (mbo.price != db::kUndefPrice) {
        LevelOrders level = {mbo};
        levels.emplace(mbo.price, level);
      }
    } else {
      LevelOrders& level = GetOrInsertLevel(mbo.side, mbo.price);
      level.emplace_back(mbo);
      auto res = orders_by_id_.emplace(mbo.order_id,
                                       PriceAndSide{mbo.price, mbo.side});
      if (!res.second) {
        throw std::invalid_argument{"Received duplicated order ID " +
                                    std::to_string(mbo.order_id)};
      }
    }
  }