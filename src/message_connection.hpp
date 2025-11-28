#ifndef MESSAGE_CONNECTION
#define MESSAGE_CONNECTION

#include "config.hpp"
#include "socket.hpp"

#include <cstdint>
#include <deque>
#include <mutex>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

class Server;

class MessageConnection{
public:
    int loss_send; //number of messages lost in sending process
    int loss_recv; //number of messages lost in receiving process
    int msgs_sent; //number of messages received onto queue from TCP
    int msgs_inp; //number of messages pushed onto queue awaiting TCP transmission
    int max_to_send_sz{0};
    int cntr{0};
    deque<MboMsg> to_send;
    deque<MboMsg> from_server;
    vector<MboMsg> send_buffer;
    vector<MboMsg> recv_buffer;
    mutex send_mutex; //protects send_buffer + actual sending
    mutex to_mutex;
    mutex from_mutex;
    Socket socket;
    Server *server;
    MessageConnection();
    MessageConnection(int socket_desc, Server *server);
    bool write_n(int file_desc, const void *buf, size_t n);
    bool read_n(int file_desc, void* buf, size_t n);
    bool send_frame(int file_desc, const void *data, uint32_t len);
    bool recv_frame(int file_desc, void *buf, uint32_t& bytes_recv);
    bool push_onto_queue(vector<MboMsg> &messages);
    void send_messages(bool last);
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