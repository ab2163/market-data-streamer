#include "data_streamer.hpp"

#include <iostream>

int main(int argc, char *argv[]){
    //file path missing
    if(argc < 2){
        cerr << "Server Script: path to DBN file missing.\n";
        return 1;
    }
    string filepath = argv[1];

    DataStreamer streamer(filepath);
    streamer.stream_messages();
    int loss_send = streamer.server.mssg_conns[0]->loss_send;

    /*
    cout << "\n===== SERVER STATS =====\n";
    cout << "Total messages sent : " << total_msgs << "\n";
    cout << "Total messages lost (recv)         : " << total_lost << "\n";
    cout << "Average per-client freq (msg/s)    : " << avg_freq << "\n";
    cout << "Global freq (msgs / max_duration)  : " << global_freq << " msg/s\n";
    */

    cout << "Num. messages lost in sending: " << loss_send << endl;
    return 0;
}