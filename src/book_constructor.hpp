#ifndef BOOK_CONSTRUCTOR
#define BOOK_CONSTRUCTOR

#include "client.hpp"

#include <databento/historical.hpp>

using namespace databento;

class BookConstructor{
public:
    Client client;
    BookConstructor();
    void build_order_book();
    void process_mbo_mssg(MboMsg &msg);
};

#endif //BOOK_CONSTRUCTOR