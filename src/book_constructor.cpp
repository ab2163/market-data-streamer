#include "client.hpp"
#include "framing.hpp"

#include <iostream>
#include <chrono>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

int main(void){
    try{
        //connect to server
        int socket_desc = connect_to("127.0.0.1", 9000);

        //buffer for receiving data from server
        const size_t BATCH_SIZE = 500;
        vector<MboMsg> buffer(BATCH_SIZE);
        uint32_t bytes_recv = 0;

        int msg_cnt = 0;
        auto start = chrono::high_resolution_clock::now();

        while(recv_frame(socket_desc, buffer.data(), bytes_recv)){
            size_t num_msgs = bytes_recv / sizeof(MboMsg);

            for(size_t i = 0; i < num_msgs; i++){
                //actions on messages here
                msg_cnt++;
            }
        }

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::microseconds>(end - start);
        cout << "Time taken: " << duration.count() << " microseconds\n";
        cout << "Received messages: " << msg_cnt << endl;

        ::shutdown(socket_desc, SHUT_RDWR); //stop sending and receiving
        ::close(socket_desc); //closes the file descriptor
    }catch(const exception& e){
        cerr << "Client error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}