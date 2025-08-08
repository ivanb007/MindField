// threadpool.h

#pragma once

#include "search.h"
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

// ThreadPool class to manage a pool of worker threads for parallel task execution.
// It allows tasks to be enqueued and executed asynchronously.
class ThreadPool {
public:
    ThreadPool(size_t n);
    ~ThreadPool();

    template<class F>
    // Enqueue a task to be executed by the thread pool.
    // Returns a future that can be used to retrieve the result of the task.
    auto enqueue(F&& f) -> std::future<decltype(f())> {
        using return_type = decltype(f());
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

private:
    // Vector to store worker threads that will execute tasks.
    std::vector<std::thread> workers;
    // Queue to hold tasks that are waiting to be executed.
    // Using a queue allows tasks to be processed in the order they are added.
    std::queue<std::function<void()>> tasks;
    // Mutex to protect access to the task queue.
    // This ensures that only one thread can modify the queue at a time.
    std::mutex queue_mutex;
    // Condition variable to notify worker threads when tasks are available.
    // This allows threads to wait for tasks without busy-waiting.
    std::condition_variable condition;
    // Flag to indicate whether the thread pool should stop accepting new tasks.
    // When set to true, worker threads will exit after completing their current tasks.
    bool stop;
};
