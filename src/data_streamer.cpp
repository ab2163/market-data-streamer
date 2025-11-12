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
        std::cout << "Server listening on: 9000\n";

        sockaddr_in peer{}; socklen_t plen = sizeof(peer); //IP and port information of peer
        int client_sock = ::accept(listen_sock, (sockaddr*)&peer, &plen); //blocks until client connects
        if(client_sock < 0) throw std::system_error(errno, std::generic_category(), "Error accepting TCP connection");
        set_socket_opts(client_sock);

        //send over all messages from databento file
        const Record *record;
        while(record = file_store.NextRecord()){
            send_frame(client_sock, static_cast<const void*>(record), 56);
        }

        ::close(client_sock);
        ::close(listen_sock);
    }catch(const std::exception& e){
        std::cerr << "Server error: " << e.what() << "\n";
        return 1;
    }

    /*
    cout << file_store.GetMetadata() << '\n';
    const Record *record = file_store.NextRecord();
    cout << "Size: " << record->Size() << endl;
    const auto& mbo_msg = record->Get<MboMsg>();
    cout << "Price: " << mbo_msg.price << endl;
    cout << "Size: " << mbo_msg.size << endl;
    cout << sizeof(mbo_msg) << endl;
    */

    return 0;
}