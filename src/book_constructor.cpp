#include "book_constructor.hpp"

BookConstructor::BookConstructor(){
    client.connect_to("127.0.0.1", 9000);
}

void BookConstructor::process_mbo_mssg(MboMsg &msg){
    //tbd
}

void BookConstructor::build_order_book(){
    while(client.receive_messages())
        client.process_messages([this](MboMsg &msg){ process_mbo_mssg(msg); });
}

