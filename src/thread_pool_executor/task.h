#pragma once
#include <future>
#include <concepts>
#include "src/common.h"

namespace algo {

class task_model {
public:
    virtual void operator()() = 0;

    virtual ~task_model() = default;
};

template<typename R>
class task : public task_model {
private:
    std::packaged_task<R()> staged_task;

public:
    template <typename F, typename ... Args>
    task(F&& f, Args&&... args) requires std::is_invocable_r_v<R, F, Args...> {
        staged_task = std::packaged_task<R()>(
            [f = std::forward<F>(f),
                args_tuple = std::tuple<Args...>(std::forward<Args>(args)...)]() mutable {
                return std::apply([&f](auto&... args) {
                    return f(std::forward<Args>(args)...);
                }, args_tuple);
            }
        );
    }

    task(task&& other) {
        staged_task = std::move(other.staged_task);
    }

    virtual void operator()() {
        staged_task();
    }

    std::future<R> get_future() {
        return staged_task.get_future();
    }
};

class task_wrapper {
public:
    std::unique_ptr<task_model> underlying_task;

    task_wrapper(std::unique_ptr<task_model>&& underlying_task) {
        this -> underlying_task = std::move(underlying_task);
    }

    template <typename F, typename ...Args>
    task_wrapper(F&& f, Args&&... args) requires std::invocable<F, Args...> {
        using R = std::invoke_result_t<F, Args...>;
        task_model* task_interface_ptr = new task<R>(std::forward<F>(f), std::forward<Args>(args)...);
        underlying_task = std::unique_ptr<task_model>(task_interface_ptr);
    }

    task_wrapper(const task_wrapper& other) = delete;

    task_wrapper(task_wrapper&& other) {
        underlying_task = std::move(other.underlying_task);
    }

    void operator()() {
        underlying_task -> operator()();
    }
};
}
