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
    return 0;
}