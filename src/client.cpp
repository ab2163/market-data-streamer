#include "client.hpp"

//creates TCP socket and connects to remote host
void Client::connect_to(const char *host, uint16_t port){
    msg_conn.socket.connect(host, port);
}

bool Client::receive_messages(){
    return msg_conn.recv_onto_queue();
}