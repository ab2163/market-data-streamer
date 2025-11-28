#include "server.hpp"

Server::Server() : thread_pool(1){
    running = false;
    num_conns = 0;
    finished_sending = false;
}

void Server::start_listening(){
    if(running) return;
    running = true;

    listen_conn.socket.bind(9000);
    listen_conn.socket.listen();
    sockaddr_in peer{}; 
    socklen_t plen = sizeof(peer);
    int client_sock = ::accept(listen_conn.socket.socket_desc, (sockaddr*)&peer, &plen);
    if(client_sock >= 0){
        lock_guard<mutex> lock(mssg_conns_mutex);
        mssg_conns.push_back(make_unique<MessageConnection>(client_sock, this));
        num_conns++;
    }
        
    accept_thread = thread([this](){
        while(running){
            sockaddr_in peer{}; 
            socklen_t plen = sizeof(peer);
            int client_sock = ::accept(listen_conn.socket.socket_desc, (sockaddr*)&peer, &plen);
            if(client_sock < 0) continue; //errors fail to produce connections
            {
                lock_guard<mutex> lock(mssg_conns_mutex);
                mssg_conns.push_back(make_unique<MessageConnection>(client_sock, this)); //constructor arguments are copied directly
                num_conns++;
            }
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

void Server::broadcast(vector<MboMsg> &messages, bool last){
    vector<MessageConnection*> snapshot;
    {
        lock_guard<mutex> lock(mssg_conns_mutex);
        snapshot.reserve(mssg_conns.size());
        for(auto& conn : mssg_conns) snapshot.push_back(conn.get());
    }

    for(auto conn_ptr : snapshot){
        conn_ptr->push_onto_queue(messages);
        //now add the task of sending the data
        thread_pool.enqueue([conn_ptr, last](){ conn_ptr->send_messages(last); });
    }
}

void Server::connection_finished(){
    int prev = num_conns.fetch_sub(1, memory_order_acq_rel);
    if(prev == 1){
        {
            lock_guard<mutex> lock(finished_mutex);
            finished_sending = true;
        }
        finished_cv.notify_all();
    }
}

void Server::wait_to_finish(){
    unique_lock<mutex> lock(finished_mutex);
    finished_cv.wait(lock, [&]{ return finished_sending; });
}