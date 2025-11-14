#ifndef DATA_STREAMER
#define DATA_STREAMER

#include "server.hpp"

#include <string>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

class DataStreamer{
public:
    static const size_t BATCH_SIZE = 500;
    DbnFileStore file_store;
    Server server;
    DataStreamer(string filepath);
    ~DataStreamer() = default;
    void stream_messages();
};

#endif //DATA_STREAMER