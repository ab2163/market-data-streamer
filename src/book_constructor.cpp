#include "client.hpp"
#include "framing.hpp"

#include <iostream>
#include <chrono>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

int main(void){
    try{
        int socket_desc = connect_to("127.0.0.1", 9000);
        MboMsg *msg = new MboMsg;

        int msg_cnt = 0;
        auto start = chrono::high_resolution_clock::now();
        while(recv_frame(socket_desc, static_cast<void*>(msg))){
            //actions on messages here
            msg_cnt++;
            /*
            if(!(msg_cnt % 10000)){
                cout << "Message details:\n";
                cout << "Price: " << msg->price << endl;
                cout << "Size: " << msg->size << endl;
                cout << "Action: " << msg->action << endl << endl;
            }
                */
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