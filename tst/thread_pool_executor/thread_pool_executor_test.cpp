#include "src/thread_pool_executor/thread_pool_executor.h"
#include "src/thread_pool_executor/task.h"
#include "gtest/gtest.h"
#include "src/vector/vector.h"

namespace {
    using namespace algo;
    class thread_pool_executor_test : public ::testing::Test {
    };

    constexpr static int start = 30;
    constexpr static int threshold = 15;

    int dp_fibonacci(int n) {
        std::vector<int> dp(1, 0);
        dp.push_back(1);
        dp.push_back(1);
        while (n >= (int) dp.size()) {
            dp.push_back(dp[dp.size() - 2] + dp.back());
        }
        return dp[n];
    }

    int fibonacci(int n) {
        if (n == 1 || n == 2) {
            return 1;
        }
        return fibonacci(n - 1) + fibonacci(n - 2);
    }

    int branching_fibonacci(int n, thread_pool_executor& executor) {
        if (n == 1 || n == 2) {
            return 1;
        }
        if (n <= threshold) {
            return fibonacci(n);
        }
        task<int> fibonacci_task1(branching_fibonacci, n - 1, executor);
        std::future<int> future1 = fibonacci_task1.get_future();
        task<int> fibonacci_task2(branching_fibonacci, n - 2, executor);
        std::future<int> future2 = fibonacci_task2.get_future();
        executor.attempt_parallel(std::move(fibonacci_task1));
        executor.attempt_parallel(std::move(fibonacci_task2));
        return future1.get() + future2.get();
    }
    
    TEST_F(thread_pool_executor_test, basic_test) {
        int repeat = 35;
        thread_pool_executor executor(1);
        vector<std::future<int> > futures;
        for (int i = 0; i < repeat; ++i) {
            int res = fibonacci(start);
            EXPECT_EQ(res, dp_fibonacci(start));
        }
    }

    TEST_F(thread_pool_executor_test, basic_parallel_test) {
        int repeat = 35;
        thread_pool_executor executor;
        vector<std::future<int> > futures;
        for (int i = 0; i < repeat; ++i) {
            task<int> fibonacci_task(fibonacci, start);
            std::future<int> future = fibonacci_task.get_future();
            futures.push_back(std::move(future));
            executor.execute(std::move(fibonacci_task));
        }
        for (int i = 0; i < repeat; ++i) {
            int res = futures[i].get();
            EXPECT_EQ(res, dp_fibonacci(start));
        }
    }

    TEST_F(thread_pool_executor_test, attempt_parallel_test) {
        thread_pool_executor executor;
        task<int> fibonacci_task(branching_fibonacci, start, executor);
        std::future<int> future = fibonacci_task.get_future();
        executor.attempt_parallel(std::move(fibonacci_task));
        int res = future.get();
        EXPECT_EQ(res, dp_fibonacci(start));
    }
}