// threadpool.cpp

#include "threadpool.h"

// Constructor to initialize the thread pool with a given number of threads.
ThreadPool::ThreadPool(size_t n) : stop(false) {
    // Create n worker threads
    for (size_t i = 0; i < n; ++i)
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                // Lock the queue mutex to ensure thread-safe access to the task queue
                // Unlock the queue before executing the task so that other threads can perform enqueue tasks
                {
                    // Lock the queue so that data can be shared safely
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    // Wait until there is a task to execute or the pool is stopped
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    // If the pool is stopped and there are no tasks, exit the thread
                    if (stop && tasks.empty()) return;
                    // Get the next task from the queue
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
}

// Destructor to stop the thread pool.
ThreadPool::~ThreadPool() {
    {
        // Lock the queue to update the stop flag safely.
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    // Notify all threads to wake up and check the stop condition.
    // This will allow them to exit if they are waiting for tasks.
    condition.notify_all();
    // Join all worker threads to ensure they finish task execution before the destructor returns.
    for (std::thread &worker : workers)
        worker.join();
}

// Template member function definitions should be in the header file, so removed from cpp.
