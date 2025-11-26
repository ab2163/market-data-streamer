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
    int msgs_sent = builder.client.msg_conn.msgs_sent;
    cout << "Num. messages lost in receiving: " << loss_recv << endl;
    cout << "Total num. messages sent: " << msgs_sent << endl;
    // Convert total duration to microseconds as a double
    double total_us = chrono::duration<double, micro>(duration).count();
    double time_per_msg = total_us / msgs_sent;      //microseconds per message
    double processing_freq = 1'000'000.0 / time_per_msg; //messages per second
    cout << "Message processing frequency: " << processing_freq << endl;
    return 0;
}