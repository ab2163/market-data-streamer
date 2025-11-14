#include "data_streamer.hpp"
#include "server.hpp"

#include <string>
#include <vector>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

DataStreamer::DataStreamer(string filepath) : file_store(filepath){
    server.start_listening();
}

void DataStreamer::stream_messages(){
    vector<MboMsg> stream_batch;
    stream_batch.reserve(BATCH_SIZE);
    while(const Record *record = file_store.NextRecord()){
        const auto& mbo_msg = record->Get<MboMsg>();
        stream_batch.push_back(mbo_msg);
        if(stream_batch.size() == BATCH_SIZE){
            server.broadcast(stream_batch);
            stream_batch.clear();
        }
    }
    if(!stream_batch.empty()) server.broadcast(stream_batch);
}