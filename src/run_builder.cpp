#include "book_constructor.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib> 

using namespace std;
using namespace std::chrono;

struct ClientStats{
    int msgs_sent = 0;
    int loss_recv = 0;
    double duration_us = 0.0;      //microseconds
    double processing_freq = 0.0;  //messages per second
};

int main(int argc, char* argv[]){
    int N = 1; //number of clients you want to simulate
    if(argc >= 2){
        N = atoi(argv[1]);
        if(N <= 0){
            cerr << "Client Script: invalid number of clients: " << argv[1] << "\n";
            return 1;
        }
    }
    cout << "Starting multi-client builder benchmark with N = " << N << "\n";

    vector<ClientStats> stats(N);
    vector<thread> threads;
    threads.reserve(N);

    //launch N client threads
    for(int i = 0; i < N; ++i){
        threads.emplace_back([i, &stats](){
            cout << "[Client " << i << "] Starting builder.\n";

            auto start = high_resolution_clock::now();

            BookConstructor builder;          //this connects to the server
            builder.build_order_book();       //this reads/constructs the book

            auto end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            double total_us = static_cast<double>(duration.count());

            int loss_recv = builder.client.msg_conn.loss_recv;
            int msgs_sent = builder.client.msg_conn.msgs_sent;

            double time_per_msg = (msgs_sent > 0)
                ? total_us / msgs_sent
                : 0.0;

            double processing_freq = (time_per_msg > 0.0)
                ? 1'000'000.0 / time_per_msg
                : 0.0;

            ClientStats s;
            s.msgs_sent = msgs_sent;
            s.loss_recv = loss_recv;
            s.duration_us = total_us;
            s.processing_freq = processing_freq;

            stats[i] = s;

            cout << "[Client " << i << "] Finished. "
                 << "msgs_sent=" << msgs_sent
                 << " loss_recv=" << loss_recv
                 << " freq=" << processing_freq << " msg/s\n";
        });
    }

    //wait for all clients to finish
    for(auto& t : threads){
        t.join();
    }

    //aggregate stats
    long long total_msgs = 0;
    long long total_lost = 0;
    double sum_freq = 0.0;
    double max_duration_us = 0.0;

    for(int i = 0; i < N; ++i){
        total_msgs += stats[i].msgs_sent;
        total_lost += stats[i].loss_recv;
        sum_freq += stats[i].processing_freq;
        if(stats[i].duration_us > max_duration_us){
            max_duration_us = stats[i].duration_us;
        }
    }

    double avg_freq = (N > 0) ? sum_freq / N : 0.0;
    double global_freq = (max_duration_us > 0.0)
        ? (total_msgs * 1'000'000.0) / max_duration_us
        : 0.0;

    cout << "\n===== AGGREGATED STATS =====\n";
    cout << "Total messages sent by all clients : " << total_msgs << "\n";
    cout << "Total messages lost (recv)         : " << total_lost << "\n";
    cout << "Average per-client freq (msg/s)    : " << avg_freq << "\n";
    cout << "Global freq (msgs / max_duration)  : " << global_freq << " msg/s\n";

    return 0;
}
