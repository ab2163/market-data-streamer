#include "server.hpp"

#include "tcp_common.hpp"
#include "framing.hpp"

#include <iostream>

int listen_on(uint16_t port){
    int socket_desc = ::socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0) throw std::system_error(errno, std::generic_category(), "Error creating socket");

    int yes = 1;
    ::setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)); //enable quick rebinding after restarting

    sockaddr_in addr{}; addr.sin_family = AF_INET; //using IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //bind to all network interfaces on this machine
    addr.sin_port = htons(port); //set port number (converted to big-endian)

    if(::bind(socket_desc, (sockaddr*)&addr, sizeof(addr)) < 0) //bind socket to IP address and port
        throw std::system_error(errno, std::generic_category(), "Error binding socket");

    if(::listen(socket_desc, 128) < 0) //socket is passive (listening) with up to 128 pending connections
        throw std::system_error(errno, std::generic_category(), "Error converting socket to listener");

    return socket_desc;
}

void set_socket_opts(int file_desc){
    int one = 1;
    ::setsockopt(file_desc, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); //disable Nagle algorithm (batching packets)
    ::setsockopt(file_desc, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)); //keepalive messages sent when connection idle
    //send and receive timeouts
    timeval tv{.tv_sec = 10, .tv_usec = 0};
    ::setsockopt(file_desc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(file_desc, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

/*--------------------------------------------------------------------------------------------------------------*/
#include "server.hpp"

Server::Server() : thread_pool(thread::hardware_concurrency()){
    running = false;
}

void Server::start_listening(){
    if(running) return;
    running = true;
        
    accept_thread = thread([this](){
        listen_conn.socket.bind(9000);
        listen_conn.socket.listen();
        while(running){
            sockaddr_in peer{}; 
            socklen_t plen = sizeof(peer);
            int client_sock = ::accept(listen_conn.socket.socket_desc, (sockaddr*)&peer, &plen);
            if(client_sock < 0) continue; //errors fail to produce connections
            mssg_conns.emplace_back(socket_desc); //constructor arguments are copied directly
        }
    });
}

void Server::stop_listening(){
    if(!running) return;
    running = false;
        
    //close connection to unblock ::accept()
    shutdown(listen_conn.socket.socket_desc, SHUT_RDWR);
        
    //wait for thread to finish
    if(accept_thread.joinable())
        accept_thread.join();
}

Server::~Server(){
    stop_listening();
}

void Server::broadcast(vector<MboMsg> &messages){
    for(auto &conn : mssg_conns){
        conn.push_onto_queue(messages);
        //now add the task of sending the data
        thread_pool.enqueue([&conn](){ conn.send_messages(); });
    }
}