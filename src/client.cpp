#include "client.hpp"

#include "tcp_common.hpp"
#include "framing.hpp"

#include <iostream>

//creates TCP socket and connects to remote host
int connect_to(const char *host, uint16_t port){
    int socket_desc = ::socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0) throw std::system_error(errno, std::generic_category(), "Error creating socket");

    sockaddr_in addr{}; addr.sin_family = AF_INET; //connecting to IPv4 address
    addr.sin_port = htons(port); //convert destination port number to big-endian and store
    if(::inet_pton(AF_INET, host, &addr.sin_addr) != 1) //checks valid IP address
        throw std::runtime_error("inet_pton failed (use IPv4 like 127.0.0.1)");

    if(::connect(socket_desc, (sockaddr*)&addr, sizeof(addr)) < 0) //perform 3-way TCP handshake (uses ephemeral port number)
        throw std::system_error(errno, std::generic_category(), "Error connecting to server");

    int one = 1;
    ::setsockopt(socket_desc, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); //disable Nagle algorithm
    return socket_desc;
}

/*--------------------------------------------------------------------------------------------------------------*/

//creates TCP socket and connects to remote host
void Client::connect_to(const char *host, uint16_t port){
    msg_conn.socket.connect(host, port);
}