#ifndef MESSAGE_CONNECTION
#define MESSAGE_CONNECTION

#include "socket.hpp"

#include <cstdint>
#include <deque>
#include <mutex>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

class MessageConnection{
public:
    static constexpr size_t MAX_QUEUE_SIZE = 50000;
    static constexpr size_t BATCH_SIZE = 500;
    deque<MboMsg> to_send;
    deque<MboMsg> from_server;
    vector<MboMsg> send_buffer;
    vector<MboMsg> recv_buffer;
    mutex to_mutex;
    mutex from_mutex;
    Socket socket;
    Server &server;
    MessageConnection();
    MessageConnection(int socket_desc, Server &server);
    void write_n(int file_desc, const void *buf, size_t n);
    bool read_n(int file_desc, void* buf, size_t n);
    void send_frame(int file_desc, const void *data, uint32_t len);
    bool recv_frame(int file_desc, void *buf, uint32_t& bytes_recv);
    bool push_onto_queue(vector<MboMsg> &messages);
    void send_messages();
    bool recv_onto_queue();
    
    template <typename F>
    //process all messages in queue
    void process_messages(F&& fn){
        lock_guard<mutex> lock(from_mutex);
        while(!from_server.empty()){
            fn(from_server.front());
            from_server.pop_front();
        }
    }
};

#endif //MESSAGE_CONNECTION