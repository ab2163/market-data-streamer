#include "tcp_common.hpp"
#include "framing.hpp"
#include <iostream>

static int listen_on(uint16_t port){
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

static void set_socket_opts(int file_desc){
    int one = 1;
    ::setsockopt(file_desc, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one)); //disable Nagle algorithm (batching packets)
    ::setsockopt(file_desc, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one)); //keepalive messages sent when connection idle
    //send and receive timeouts
    timeval tv{.tv_sec = 10, .tv_usec = 0};
    ::setsockopt(file_desc, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(file_desc, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

int main(){
    try{
        int listen_sock = listen_on(9000);
        std::cout << "Server listening on: 9000\n";

        sockaddr_in peer{}; socklen_t plen = sizeof(peer); //IP and port information of peer
        int client_sock = ::accept(listen_sock, (sockaddr*)&peer, &plen); //blocks until client connects
        if(client_sock < 0) throw std::system_error(errno, std::generic_category(), "Error accepting TCP connection");

        set_socket_opts(client_sock);

        //example loop: echo any frame back uppercased
        while(true){
            std::string frame = recv_frame(client_sock);
            for(char& c : frame) c = std::toupper(static_cast<unsigned char>(c));
            send_frame(client_sock, frame.data(), static_cast<uint32_t>(frame.size()));
        }

        ::close(client_sock);
        ::close(listen_sock);
    }catch(const std::exception& e){
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }
}
