#include "order_book.hpp"

#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace databento;

void OrderBook::update_book(MboMsg &msg){
    //cout << to_string(msg.order_id) << endl;

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
            cerr << "Unknown action: " << ToString(msg.action) << endl;
    }
}

void OrderBook::clear_book(){
    orders_by_id.clear();
    bid_orders.clear();
    ask_orders.clear();
}

OrderBook::SideLevels& OrderBook::get_side_levels(Side side){
    if(side == Side::Bid) return bid_orders;
    else return ask_orders;
}

void OrderBook::add_order(MboMsg &msg){
    SideLevels &levels = get_side_levels(msg.side);
    if(msg.flags.IsTob()){ //top-of-book "refresh"
        levels.clear();
        if(msg.price != kUndefPrice){ //insert "synthetic" TOP order
            LevelOrders level = {msg};
            levels.emplace(msg.price, level);
        }
    }
    else{
        if(orders_by_id.find(msg.order_id) != orders_by_id.end()){
            //duplicate Add – ignore to avoid corrupting state further
            cerr << "OrderBook: duplicate add for order: " << to_string(msg.order_id) << endl;
            return;
        }

        levels[msg.price].emplace_back(msg); //put price into correct level
        auto res = orders_by_id.emplace(msg.order_id, PriceAndSide{msg.price, msg.side}); //insert into orders by ID
    }
}

void OrderBook::cancel_order(MboMsg &msg){
    auto id_it = orders_by_id.find(msg.order_id);
    //if(id_it == orders_by_id.end()) throw invalid_argument{ "Cancel for unknown order ID: " + to_string(msg.order_id) };
    if(id_it == orders_by_id.end()) return; //allow false cancellations without causing crash
    auto prAndSide = id_it->second;

    auto &levels = get_side_levels(prAndSide.side);

    auto level_it = levels.find(prAndSide.price);
    //if the order exists in orders_by_id but not in levels then just remove from orders_by_id
    //this may be because a previous add_order with IsTob() happened
    if(level_it == levels.end()){ 
        orders_by_id.erase(id_it);
        return;
    }
    auto &level = level_it->second;

    auto it = find_if(level.begin(), level.end(), [&](auto &level_msg){ return level_msg.order_id == msg.order_id; });
    if(it == level.end()){
        //mapping exists but we can't find the order in the level
        //drop the mapping and move on (don't crash)
        orders_by_id.erase(id_it);
        if(level.empty()) levels.erase(level_it);
        return;
    }

    if(it->size < msg.size) it->size = 0; //if size to cancel > order size then clamp to 0 (don't crash)
    it->size -= msg.size;
    if(it->size == 0){
        orders_by_id.erase(id_it);
        level.erase(it);
        if(level.empty()) levels.erase(level_it);
    }
}

void OrderBook::modify_order(MboMsg &msg){
    auto id_it = orders_by_id.find(msg.order_id);
    if(id_it == orders_by_id.end()){
        //order not found treated as new order
        add_order(msg);
        return;
    }
    
    auto prev_side = id_it->second.side;
    auto prev_price = id_it->second.price;

    auto &levels = get_side_levels(prev_side);
    auto level_map_it = levels.find(prev_price);

    if(level_map_it == levels.end()){
        //mapping exists but level doesn't – state is broken
        //best effort: drop old mapping and treat this as fresh add
        orders_by_id.erase(id_it);
        add_order(msg);
        return;
    }

    auto &prev_level = level_map_it->second;
    auto level_it = find_if(prev_level.begin(), prev_level.end(), 
        [&](auto &level_msg){ return level_msg.order_id == msg.order_id; });

    if(level_it == prev_level.end()){
        //mapping exists but order missing – treat as fresh add
        orders_by_id.erase(id_it);
        add_order(msg);
        return;
    }

    //if side changed, interpret it as "delete old + add new"
    if(prev_side != msg.side){
        prev_level.erase(level_it);
        if(prev_level.empty()){
            levels.erase(level_map_it);
        }
        orders_by_id.erase(id_it);
        add_order(msg);
        return;
    }

    //now we know: same side, existing order found

    if(prev_price != msg.price){
        //price changed means loses priority
        id_it->second.price = msg.price;
        prev_level.erase(level_it);
        if(prev_level.empty()) levels.erase(level_map_it);
        levels[msg.price].emplace_back(msg);
    }
    else if(level_it->size < msg.size){
        //size increased means loses priority
        auto &level = prev_level;
        level.erase(level_it);
        level.emplace_back(msg);
    }
    else{
        //otherwise same price with smaller size means keeps priority
        level_it->size = msg.size;
    }
}

PriceLevel OrderBook::get_price_level(int64_t price, const LevelOrders &level){
    PriceLevel res{price};
    for(const auto &order : level){
        if(!order.flags.IsTob()) res.count++;
        res.size += order.size;
    }
    return res;
}

PriceLevel OrderBook::get_bid_level(){
    if(bid_orders.empty()) return PriceLevel{};
    auto level_it = bid_orders.rbegin();
    return get_price_level(level_it->first, level_it->second);
}

PriceLevel OrderBook::get_ask_level(){
    if(ask_orders.empty()) return PriceLevel{};
    auto level_it = ask_orders.begin();
    return get_price_level(level_it->first, level_it->second);
}

void OrderBook::print_BBO(MboMsg &msg){
    PriceLevel bidPL = get_bid_level();
    PriceLevel askPL = get_ask_level();
    cout << "CLX5 BBO | " << ToIso8601(msg.ts_recv) << endl;
    cout << askPL.size << " @ " << pretty::Px{askPL.price} << " | " << askPL.count << " order(s)" << endl;
    cout << bidPL.size << " @ " << pretty::Px{bidPL.price} << " | " << bidPL.count << " order(s)" << endl;
    cout << endl;
}