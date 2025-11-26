#include "message_connection.hpp"
#include "server.hpp"
#include "tcp_common.hpp"

using namespace std;

//constructor used client-side and for server-side listener 
MessageConnection::MessageConnection(){
    send_buffer.reserve(BATCH_SIZE);
    recv_buffer.reserve(BATCH_SIZE);
    loss_send = loss_recv = msgs_sent = 0;
}

//constructor used server-side
MessageConnection::MessageConnection(int socket_desc, Server *server) : socket(socket_desc), server(server){
    send_buffer.reserve(BATCH_SIZE);
    recv_buffer.reserve(BATCH_SIZE);
    loss_send = loss_recv = msgs_sent = 0;
    socket.configure(Role::Server);
}

//function which writes n bytes over the connection
void MessageConnection::write_n(int file_desc, const void *buf, size_t n){
    const uint8_t *p = static_cast<const uint8_t*>(buf);
    size_t bytes_sent = 0;
    while(bytes_sent < n){
        ssize_t return_code = ::send(file_desc, p + bytes_sent, n - bytes_sent, 0);
        if(return_code < 0){
            if(errno == EINTR) continue;
            throw system_error(errno, generic_category(), "Error in sending over TCP");
        }
        if(return_code == 0) throw runtime_error("Error: connection closed by peer");
        bytes_sent += static_cast<size_t>(return_code);
    }
}

//read n bytes over the connection
bool MessageConnection::read_n(int file_desc, void* buf, size_t n){
    uint8_t* p = static_cast<uint8_t*>(buf);
    size_t bytes_recv = 0;
    while(bytes_recv < n){
        ssize_t return_code = ::recv(file_desc, p + bytes_recv, n - bytes_recv, 0);
        if(return_code < 0){
            if (errno == EINTR) continue;
            throw system_error(errno, generic_category(), "Error in receiving over TCP");
        }
        if(return_code == 0) return false;
        bytes_recv += static_cast<size_t>(return_code);
    }
    return true;
}

//send data as length-prefixed frame: [u32 length_le][payload bytes]
void MessageConnection::send_frame(int file_desc, const void *data, uint32_t len){
    //use little-endian consistently
    uint32_t len_LE = htole32(len);
    write_n(file_desc, &len_LE, sizeof(len_LE));
    write_n(file_desc, data, len);
}

//receive data as length-prefixed frame
bool MessageConnection::recv_frame(int file_desc, void *buf, uint32_t& bytes_recv){
    uint32_t len_LE = 0;
    if(!read_n(file_desc, &len_LE, sizeof(len_LE))) return false;;
    uint32_t len = le32toh(len_LE);
    if(len > BATCH_SIZE * sizeof(MboMsg))
        throw runtime_error("Frame too large for buffer");
    if(len){
        bool success = read_n(file_desc, buf, len);
        if(success) bytes_recv = len;
        return success;
    }
    else return false;
}

//pushes messages onto queue for client
bool MessageConnection::push_onto_queue(vector<MboMsg> &messages){
    lock_guard<mutex> lock(to_mutex);
    if(to_send.size() + messages.size() > MAX_QUEUE_SIZE){
        loss_send += messages.size(); //for logging purposes
        return false; //dump messages if not enough space
    }
    to_send.insert(to_send.end(), messages.begin(), messages.end());
    return true;
}

//sends a batch of messages from queue
void MessageConnection::send_messages(bool last){
    lock_guard<mutex> send_lock(send_mutex); //only one thread at a time
    {
        lock_guard<mutex> lock(to_mutex);
        if(to_send.empty()){
            if(last) server->connection_finished();
            return; //early exit on empty queue
        }
        int mssg_cnt = min(BATCH_SIZE, to_send.size());
        send_buffer.assign(to_send.begin(), to_send.begin() + mssg_cnt);
        to_send.erase(to_send.begin(), to_send.begin() + mssg_cnt);
    }
    send_frame(socket.socket_desc, send_buffer.data(), send_buffer.size()*sizeof(MboMsg));
    if(last) server->connection_finished();
}

//gets messages from receive buffer and pushes onto queue if possible
bool MessageConnection::recv_onto_queue(){
    recv_buffer.resize(BATCH_SIZE);
    uint32_t bytes_recv = 0;
    if(!recv_frame(socket.socket_desc, recv_buffer.data(), bytes_recv)) return false;
    size_t num_msgs = bytes_recv / sizeof(MboMsg);
    recv_buffer.resize(num_msgs);    //trim to actual number of messages
    lock_guard<mutex> lock(from_mutex);
    if(from_server.size() + num_msgs > MAX_QUEUE_SIZE){
        loss_recv += num_msgs; //for logging purposes
        return false; //dump messages if not enough space
    }
    msgs_sent += num_msgs; //for logging purposes
    from_server.insert(from_server.end(), recv_buffer.begin(), recv_buffer.begin() + num_msgs);
    return true;
}