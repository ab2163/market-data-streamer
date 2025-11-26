#include "book_constructor.hpp"

BookConstructor::BookConstructor(){
    client.connect_to("127.0.0.1", 9000);
    timest_cnt = 0;
}

void BookConstructor::process_mbo_mssg(MboMsg &msg){
    order_book.update_book(msg);
    timest_cnt++;
    if(msg.flags.IsLast() && !(timest_cnt % 100))
        order_book.print_BBO(msg); //periodic outputting of BBO
}

void BookConstructor::build_order_book(){
    while(client.receive_messages())
        client.process_messages([this](MboMsg &msg){ process_mbo_mssg(msg); });
}

