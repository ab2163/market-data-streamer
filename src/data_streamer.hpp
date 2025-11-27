#ifndef DATA_STREAMER
#define DATA_STREAMER

#include "config.hpp"
#include "server.hpp"

#include <string>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

class DataStreamer{
public:
    DbnFileStore file_store;
    Server server;
    DataStreamer(string filepath);
    ~DataStreamer() = default;
    void stream_messages();
};

#endif //DATA_STREAMER