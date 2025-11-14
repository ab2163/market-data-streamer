#include "book_constructor.hpp"

#include <iostream>

int main(void){
    cout << "Starting builder script.\n";
    BookConstructor builder;
    builder.build_order_book();
    cout << "Finished builder script.\n";
    return 0;
}