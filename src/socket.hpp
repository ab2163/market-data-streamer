#ifndef SOCKET
#define SOCKET

#include <cstdint>

enum class Role{
    Client,
    Server
};

class Socket{
public:
    int socket_desc;
    Socket();
    ~Socket();
    void bind(uint16_t port); //binds to port on all interfaces
    void listen(); //converts socket to listener
    void connect(const char *host, uint16_t port);
    void configure(Role socket_role);
};

#endif //SOCKET