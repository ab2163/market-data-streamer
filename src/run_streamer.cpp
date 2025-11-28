#include "data_streamer.hpp"

#include <iostream>
#include <cstdio>

int main(int argc, char *argv[]){
    //file path missing
    if(argc < 2){
        cerr << "Server Script: path to DBN file missing.\n";
        return 1;
    }
    string filepath = argv[1];

    DataStreamer streamer(filepath);
    streamer.stream_messages();

    int tot_loss_send = 0;
    int tot_msgs_inp = 0;
    for(auto &conn : streamer.server.mssg_conns){
        tot_loss_send += conn->loss_send;
        tot_msgs_inp += conn->msgs_inp;
    }
    double tot_drop_rate = ((double) tot_loss_send) / tot_msgs_inp;

    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f%%", tot_drop_rate);
    cout << "\n===== SERVER STATS =====\n";
    cout << "Total messages sent    : " << tot_msgs_inp << "\n";
    cout << "Total messages dropped : " << tot_loss_send << "\n";
    cout << "Message drop rate      : " << buf << "\n";

    return 0;
}