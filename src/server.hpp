#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>

int listen_on(uint16_t port);
void set_socket_opts(int file_desc);

#endif //SERVER_HPP