#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "task.h"
#include "src/vector/vector.h"

namespace algo {
class thread_pool_executor {
private:
    std::queue<task_wrapper> task_wrappers;
    std::mutex tasks_mutex;
    vector<std::thread> threads;
    std::condition_variable condvar;
    std::size_t thread_count;
    std::size_t waiting_count;
    bool alive;

public:
    thread_pool_executor(std::size_t thread_count=std::thread::hardware_concurrency())
        : thread_count(thread_count), waiting_count(0), alive(false) { }

    thread_pool_executor(const thread_pool_executor& other) = delete;

    thread_pool_executor(thread_pool_executor&& other) = delete;

    ~thread_pool_executor() {
        alive = false;
        condvar.notify_all();
        for (std::thread& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    
    template<typename R, typename ...Args>
    void execute(task<R>&& job) {
        auto task_ptr = new task<R>(std::move(job));
        std::unique_ptr<task_model> task_model_ptr(task_ptr);
        task_wrapper wrapper(std::move(task_model_ptr));
        std::unique_lock<std::mutex> tasks_unique_lock(tasks_mutex);
        if (!alive) {
            activate(tasks_unique_lock);
        }
        task_wrappers.push(std::move(wrapper));
        if (waiting_count > 0) {
            --waiting_count;
            condvar.notify_one();
        }
    }

    template<typename R, typename ...Args>
    void attempt_parallel(task<R>&& job) {
        auto task_ptr = new task<R>(std::move(job));
        std::unique_ptr<task_model> task_model_ptr(task_ptr);
        task_wrapper wrapper(std::move(task_model_ptr));
        std::unique_lock<std::mutex> tasks_unique_lock(tasks_mutex);
        if (alive && waiting_count == 0) {
            tasks_unique_lock.unlock();
            wrapper();
            return;
        }
        if (!alive) {
            activate(tasks_unique_lock);
        }
        task_wrappers.push(std::move(wrapper));
        if (waiting_count > 0) {
            --waiting_count;
            condvar.notify_one();
        }
    }

private:
    void activate(std::unique_lock<std::mutex>& lock) {
        alive = true;
        for (std::size_t i = 0; i < thread_count; ++i) {
            std::thread th(thread_func, this);
            threads.push_back(std::move(th));
        }
        condvar.wait(lock, [this](){ return waiting_count == thread_count; });
    }

    static void thread_func(thread_pool_executor* executor_ptr) {
        thread_pool_executor& executor = *executor_ptr;
        bool ready = false;
        while (executor.alive) {
            std::unique_lock<std::mutex> tasks_unique_lock(executor.tasks_mutex);
            if (!executor.task_wrappers.empty()) {
                task_wrapper task_wrapper = std::move(executor.task_wrappers.front());
                executor.task_wrappers.pop();
                tasks_unique_lock.unlock();
                task_wrapper();
            } else {
                ++executor.waiting_count;
                if (!ready) {
                    if (executor.waiting_count == executor.thread_count) {
                        executor.condvar.notify_all();
                    }
                    ready = true;
                }
                executor.condvar.wait(tasks_unique_lock, [&executor]{ return !executor.alive || !executor.task_wrappers.empty(); });
            }
        }
    }
};
}