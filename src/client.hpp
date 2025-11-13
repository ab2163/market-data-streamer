#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "message_connection.hpp"

#include <cstdint>

int connect_to(const char *host, uint16_t port);

#endif //CLIENT_HPP

/*--------------------------------------------------------------------------------------------------------------*/

#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client{
    MessageConnection msg_conn;
    connect_to(const char *host, uint16_t port);
    queue<MboMsg> from_sever;
};

#endif //CLIENT_HPP