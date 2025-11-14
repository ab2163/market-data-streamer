#ifndef THREAD_POOL
#define THREAD_POOL

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>
#include <iostream>

using namespace std;

class ThreadPool{
private:
    //worker threads
    vector<thread> workers;
    //queue of work items (any callable)
    queue<function<void()>> task_queue;
    //synchronization
    mutable mutex queue_mutex;
    condition_variable condition;
    atomic<bool> stop_flag{false};
    
    void worker_function();
    
public:
    ThreadPool(int num_threads);
    ~ThreadPool();
    int pending_tasks();
    void enqueue(function<void()> task);
    int pending_tasks() const;
};

#endif //THREAD_POOL