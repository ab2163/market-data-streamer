#include "book_constructor.hpp"

#include <iostream>
#include <chrono>

int main(void){
    cout << "Starting builder script.\n";
    auto start = chrono::high_resolution_clock::now();
    BookConstructor builder;
    builder.build_order_book();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " microseconds\n";
    cout << "Finished builder script.\n";
    int loss_recv = builder.client.msg_conn.loss_recv;
    cout << "Num. messages lost in receiving: " << loss_recv << endl;
    return 0;
}