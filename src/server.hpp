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
    atomic<int> num_conns;
    condition_variable finished_cv;
    mutex finished_mutex;
    bool finished_sending;
    Server();
    ~Server();
    void start_listening();
    void stop_listening();
    void broadcast(vector<MboMsg> &messages, bool last);
    void connection_finished();
    void wait_to_finish();
};

#endif //SERVER_HPP