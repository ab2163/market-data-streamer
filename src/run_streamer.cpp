#include "data_streamer.hpp"

#include <iostream>

int main(int argc, char *argv[]){
    cout << "Starting streamer script.\n";
    //file path missing
    if(argc < 2){
        cerr << "Path to DBN file missing.\n";
        return 1;
    }
    string filepath = argv[1];

    DataStreamer streamer(filepath);
    streamer.stream_messages();
    cout << "Ending streamer script.\n";
    int loss_send = streamer.server.mssg_conns[0]->loss_send;
    cout << "Num. messages lost in sending: " << loss_send << endl;
    return 0;
}