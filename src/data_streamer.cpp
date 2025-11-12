#include "server.hpp"
#include "framing.hpp"

#include <iostream>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

int main(int argc, char *argv[]){
    //file path missing
    if(argc < 2){
        cerr << "Path to DBN file missing.\n";
        return 1;
    }

    //open the file
    string filepath = argv[1];
    auto file_store = DbnFileStore(filepath);

    try{
        //wait for client connection
        int listen_sock = listen_on(9000);
        cout << "Server listening on: 9000\n";

        sockaddr_in peer{}; socklen_t plen = sizeof(peer); //IP and port information of peer
        int client_sock = ::accept(listen_sock, (sockaddr*)&peer, &plen); //blocks until client connects
        if(client_sock < 0) throw system_error(errno, generic_category(), "Error accepting TCP connection");
        set_socket_opts(client_sock);
        cout << "Connected to client.\n";

        //send over all messages from databento file
        const Record *record;
        while(record = file_store.NextRecord()){
            const auto& mbo_msg = record->Get<MboMsg>();
            const MboMsg *msg_ptr = addressof(mbo_msg);
            send_frame(client_sock, static_cast<const void*>(msg_ptr), 56);
        }

        ::shutdown(client_sock, SHUT_WR);
        ::close(client_sock);
        ::close(listen_sock);
    }catch(const exception& e){
        cerr << "Server error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}