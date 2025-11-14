#include "socket.hpp"

#include <unistd.h>

using namespace std;

Socket::~Socket(){
    if(socket_desc >= 0) ::close(socket_desc);
}

Socket::Socket(){
    socket_desc = ::socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0) throw system_error(errno, generic_category(), "Error creating socket");
}

void Socket::bind(uint16_t port){
    sockaddr_in addr{}; addr.sin_family = AF_INET; //using IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //bind to all network interfaces on this machine
    addr.sin_port = htons(port); //set port number (converted to big-endian)

    if(::bind(socket_desc, (sockaddr*)&addr, sizeof(addr)) < 0) //bind socket to IP address and port
        throw system_error(errno, generic_category(), "Error binding socket");
}

void Socket::listen(){
    if(::listen(socket_desc, 128) < 0) //socket is passive (listening) with up to 128 pending connections
        throw system_error(errno, generic_category(), "Error converting socket to listener");
    int yes = 1;
    ::setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); //enable quick rebinding after restarting
}

void Socket::void connect(const char *host, uint16_t port){
    sockaddr_in addr{}; addr.sin_family = AF_INET; //connecting to IPv4 address
    addr.sin_port = htons(port); //convert destination port number to big-endian and store
    if(::inet_pton(AF_INET, host, &addr.sin_addr) != 1) //checks valid IP address
        throw std::runtime_error("inet_pton failed (use IPv4 like 127.0.0.1)");

    if(::connect(socket_desc, (sockaddr*)&addr, sizeof(addr)) < 0) //perform 3-way TCP handshake (uses ephemeral port number)
        throw std::system_error(errno, std::generic_category(), "Error connecting to server");
    configure(Role::Client)
}

void Socket::configure(Role socket_role){
    int one = 1;
    ::setsockopt(file_desc, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); //disable Nagle algorithm (batching packets)
    ::setsockopt(file_desc, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)); //keepalive messages sent when connection idle
    if(Role == Role::Server){
        timeval send_tv{.tv_sec = 2, .tv_usec = 0};
        ::setsockopt(socket_desc, SOL_SOCKET, SO_SNDTIMEO, &send_tv, sizeof(send_tv));
        timeval recv_tv{.tv_sec = 30, .tv_usec = 0};
        ::setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));
    }
    else{
        timeval recv_tv{.tv_sec = 30, .tv_usec = 0};
        ::setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));
        timeval send_tv{.tv_sec = 10, .tv_usec = 0};
        ::setsockopt(socket_desc, SOL_SOCKET, SO_SNDTIMEO, &send_tv, sizeof(send_tv));
    }
}