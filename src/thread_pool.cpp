#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <atomic>

using namespace std;

//worker thread function
void ThreadPool::worker_function(){
    while(true){
        function<void()> task;
        {
            unique_lock<mutex> lock(queue_mutex);
                
            //wait for work or stop signal
            condition.wait(lock, [this]{ 
                return stop_flag || !task_queue.empty(); 
            });
                
            //exit if stopping and no more tasks
            if(stop_flag && task_queue.empty()){
                    eturn;
            }
                
            //get task from queue
            task = move(task_queue.front());
            task_queue.pop();
        }
            
        //execute task (outside the lock)
        try{
            task();
        }catch(const exception& e) {
            cerr << "Thread pool task threw exception: " << e.what() << endl;
        }
    }
}
    
//constructor: create worker threads
ThreadPool::ThreadPool(int num_threads){
    for(int i = 0; i < num_threads; i++){
        workers.emplace_back(&ThreadPool::worker_function, this);
    }
}

//destructor: stop all workers
ThreadPool::~ThreadPool(){
    stop_flag = true;
    condition.notify_all();
        
    for(auto& worker : workers){
        if(worker.joinable()){
            worker.join();
        }
    }
}
    
//number of pending tasks
int ThreadPool::pending_tasks() const{
    lock_guard<mutex> lock(queue_mutex);
    return task_queue.size();
}

//submit any callable as work
void ThreadPool::enqueue(function<void()> task){
    {
        lock_guard<mutex> lock(queue_mutex);
        task_queue.push(move(task));
    }
    condition.notify_one();
}