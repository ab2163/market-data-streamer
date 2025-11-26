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
            throw invalid_argument{ string{"Unknown action: "} + ToString(msg.action) };
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
        levels[msg.price].emplace_back(msg); //put price into correct level
        auto res = orders_by_id.emplace(msg.order_id, PriceAndSide{msg.price, msg.side}); //insert into orders by ID
        if(!res.second) throw invalid_argument{ "Received duplicated order ID " + to_string(msg.order_id) };
    }
}

void OrderBook::cancel_order(MboMsg &msg){
    auto id_it = orders_by_id.find(msg.order_id);
    //if(id_it == orders_by_id.end()) throw invalid_argument{ "Cancel for unknown order ID: " + to_string(msg.order_id) };
    if(id_it == orders_by_id.end()) return; //allow false cancellations without causing crash
    auto prAndSide = id_it->second;

    auto &levels = get_side_levels(prAndSide.side);

    auto level_it = levels.find(prAndSide.price);
    if(level_it == levels.end())
        throw logic_error{ "Internal book inconsistency: level not found for order ID " + to_string(msg.order_id) };
    auto &level = level_it->second;

    auto it = find_if(level.begin(), level.end(), [&](auto &level_msg){ return level_msg.order_id == msg.order_id; });
    if(it == level.end()) throw invalid_argument{ "Could not locate order to cancel. Order ID: " + to_string(msg.order_id) };

    if(it->size < msg.size) throw logic_error{ "Tried to cancel more size than existed." };
    it->size -= msg.size;
    if(it->size == 0){
        orders_by_id.erase(msg.order_id);
        level.erase(it);
        if(level.empty()) levels.erase(prAndSide.price);
    }
}

void OrderBook::modify_order(MboMsg &msg){
    auto it = orders_by_id.find(msg.order_id);
    if(it == orders_by_id.end()){
        //order not found treated as new order
        add_order(msg);
        return;
    }
    if(it->second.side != msg.side){
        //order cannot change side
        throw logic_error{ "Order changed side." };
    }
    auto prev_price = it->second.price;
    auto &levels = get_side_levels(it->second.side);
    auto &prev_level = levels[prev_price];
    auto level_it = find_if(prev_level.begin(), prev_level.end(), 
        [&](auto &level_msg){ return level_msg.order_id == msg.order_id; });
    if(prev_price != msg.price){
        //price changed means loses priority
        it->second.price = msg.price;
        prev_level.erase(level_it);
        if(prev_level.empty()) levels.erase(prev_price);
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
