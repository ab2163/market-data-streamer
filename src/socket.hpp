#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <cstdint>
#include <unistd.h>

enum class Role{
    Client,
    Server
};

class Socket{
public:
    int socket_desc;
    Socket();
    explicit Socket(int existing_fd);
    ~Socket();
    Socket(const Socket&) = delete; //forbid copying
    Socket& operator=(const Socket&) = delete; //forbid copying
    Socket(Socket&& other) noexcept : socket_desc(other.socket_desc){ //allow moving
        other.socket_desc = -1;
    }
    Socket& operator=(Socket&& other) noexcept{ //alow move assignment
        if(this != &other){
            if(socket_desc >= 0) ::close(socket_desc);
            socket_desc = other.socket_desc;
            other.socket_desc = -1;
        }
        return *this;
    }
    void bind(uint16_t port); //binds to port on all interfaces
    void listen(); //converts socket to listener
    void connect(const char *host, uint16_t port);
    void configure(Role socket_role);
};

#endif //SOCKET_HPP