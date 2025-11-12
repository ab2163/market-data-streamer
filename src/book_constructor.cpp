#include "client.hpp"
#include "framing.hpp"

#include <iostream>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

int main(void){
    try{
        int socket_desc = connect_to("127.0.0.1", 9000);
        //std::string msg = "hello tcp world";
        //send_frame(socket_desc, msg.data(), static_cast<uint32_t>(msg.size()));

        std::string reply = recv_frame(socket_desc);
        std::cout << "Server replied: " << reply << "\n";

        ::shutdown(socket_desc, SHUT_RDWR); //stop sending and receiving
        ::close(socket_desc); //closes the file descriptor
    }catch(const std::exception& e){
        std::cerr << "Client error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}