#ifndef BOOK_CONSTRUCTOR
#define BOOK_CONSTRUCTOR

#include "client.hpp"
#include "order_book.hpp"

#include <databento/historical.hpp>

using namespace databento;

class BookConstructor{
public:
    int timest_cnt; //count of number of "timestamps" for which data has been sent
    Client client;
    OrderBook order_book;
    BookConstructor();
    void build_order_book();
    void process_mbo_mssg(MboMsg &msg);
};

#endif //BOOK_CONSTRUCTOR