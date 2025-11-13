#ifndef MESSAGE_CONNECTION
#define MESSAGE_CONNECTION

#include "socket.hpp"

#include <cstdint>
#include <deque>

#include <databento/historical.hpp>

using namespace std;

class MessageConnection{
public:
    static const size_t MAX_QUEUE_SIZE = 50000;
    static const size_t BATCH_SIZE = 500;
    deque<MboMsg> to_send;
    vector<MboMsg> send_buffer;
    vector<MboMsg> recv_buffer;
    Socket socket;
    MessageConnection();
    void write_n(int file_desc, const void *buf, size_t n);
    bool read_n(int file_desc, void* buf, size_t n);
    void send_frame(int file_desc, const void *data, uint32_t len);
    bool recv_frame(int file_desc, void *buf, uint32_t& bytes_recv);
    bool push_onto_queue(vector<MboMsg> &messages);
    void send_messages();
};

#endif //MESSAGE_CONNECTION