#include "server.hpp"

Server::Server() : thread_pool(thread::hardware_concurrency()){
    running = false;
}

void Server::start_listening(){
    if(running) return;
    running = true;
        
    accept_thread = thread([this](){
        listen_conn.socket.bind(9000);
        listen_conn.socket.listen();
        while(running){
            sockaddr_in peer{}; 
            socklen_t plen = sizeof(peer);
            int client_sock = ::accept(listen_conn.socket.socket_desc, (sockaddr*)&peer, &plen);
            if(client_sock < 0) continue; //errors fail to produce connections
            //mssg_conns.emplace_back(client_sock); //constructor arguments are copied directly
            mssg_conns.push_back(make_unique<MessageConnection>(client_sock));
        }
    });
}

void Server::stop_listening(){
    if(!running) return;
    running = false;
        
    //close connection to unblock ::accept()
    shutdown(listen_conn.socket.socket_desc, SHUT_RDWR);
        
    //wait for thread to finish
    if(accept_thread.joinable())
        accept_thread.join();
}

Server::~Server(){
    stop_listening();
}

void Server::broadcast(vector<MboMsg> &messages){
    for(auto &conn : mssg_conns){
        conn->push_onto_queue(messages);
        //now add the task of sending the data
        thread_pool.enqueue([conn_ptr = conn.get()](){ conn_ptr->send_messages(); });
    }
}