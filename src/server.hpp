#ifndef SERVER_HPP
#define SERVER_HPP

#include "message_connection.hpp"
#include "thread_pool.hpp"

#include <vector>
#include <thread>
#include <atomic>

#include <databento/historical.hpp>

using namespace std;
using namespace databento;

class Server{
public:
    int num_threads;
    ThreadPool thread_pool;
    MessageConnection listen_conn;
    vector<unique_ptr<MessageConnection>> mssg_conns;
    thread accept_thread;
    atomic<bool> running;
    Server();
    ~Server();
    void start_listening();
    void stop_listening();
    void broadcast(vector<MboMsg> &messages);
};

#endif //SERVER_HPP