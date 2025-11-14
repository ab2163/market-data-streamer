#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "message_connection.hpp"

#include <cstdint>

int connect_to(const char *host, uint16_t port);

#endif //CLIENT_HPP

/*--------------------------------------------------------------------------------------------------------------*/

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "message_connection.hpp"

#include <cstdint>

class Client{
    MessageConnection msg_conn;
    void connect_to(const char *host, uint16_t port);
    bool receive_messages();
    void process_messages(F&& fn){
        msg_conn.process_messages(fn);
    }
};

#endif //CLIENT_HPP