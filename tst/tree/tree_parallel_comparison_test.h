#pragma once
#include "gtest/gtest.h"
#include "tst/utility/common.h"
#include "src/vector.h"
#include "src/thread_pool_executor/task.h"

namespace {
    using namespace algo;

    static const int SIZE = 100000;

    template <typename T>
    class tree_parallel_comparison_test : public testing::Test {
        public:
        inline static vector<T> trees1;
        inline static vector<T> trees2;
        inline static int index;
        static void SetUpTestSuite() {
            auto stubs = get_random_stub_vector(SIZE, 0, SIZE);
            thread_pool_executor executor;
            vector<std::future<T> > futures1;
            vector<std::future<T> > futures2;
            for (int i = 0; i < 6; i++) {
                task<T> tree1_gen_task([&stubs]() {
                    return T(stubs.begin(), stubs.begin() + stubs.size() * 2/3);
                });
                task<T> tree2_gen_task([&stubs]() {
                    return T(stubs.rbegin(), stubs.rbegin() + stubs.size() * 2/3);
                });
                futures1.push_back(tree1_gen_task.get_future());
                futures2.push_back(tree2_gen_task.get_future());
                executor.execute(std::move(tree1_gen_task));
                executor.execute(std::move(tree2_gen_task));
            }
            for (int i = 0; i < 6; i++) {
                trees1.push_back(futures1[i].get());
                trees2.push_back(futures2[i].get());
            }
            index = 0;
        }

        static void TearDownTestSuite() { }
    };

    TYPED_TEST_SUITE_P(tree_parallel_comparison_test);

    TYPED_TEST_P(tree_parallel_comparison_test, union_of_test) {
        TypeParam tree1(std::move(this -> trees1[this -> index]));
        TypeParam tree2(std::move(this -> trees2[this -> index]));
        this -> index++;
        union_of(std::move(tree1), std::move(tree2), uid_resolver());
    }

    TYPED_TEST_P(tree_parallel_comparison_test, union_of_parallel_test) {
        TypeParam tree1(std::move(this -> trees1[this -> index]));
        TypeParam tree2(std::move(this -> trees2[this -> index]));
        this -> index++;
        thread_pool_executor executor;
        union_of(std::move(tree1), std::move(tree2), executor, uid_resolver());
    }

    TYPED_TEST_P(tree_parallel_comparison_test, intersection_of_test) {
        TypeParam tree1(std::move(this -> trees1[this -> index]));
        TypeParam tree2(std::move(this -> trees2[this -> index]));
        this -> index++;
        intersection_of(std::move(tree1), std::move(tree2), uid_resolver());
    }

    TYPED_TEST_P(tree_parallel_comparison_test, intersection_of_parallel_test) {
        TypeParam tree1(std::move(this -> trees1[this -> index]));
        TypeParam tree2(std::move(this -> trees2[this -> index]));
        this -> index++;
        thread_pool_executor executor;
        intersection_of(std::move(tree1), std::move(tree2), executor, uid_resolver());
    }

    TYPED_TEST_P(tree_parallel_comparison_test, difference_of_test) {
        TypeParam tree1(std::move(this -> trees1[this -> index]));
        TypeParam tree2(std::move(this -> trees2[this -> index]));
        this -> index++;
        difference_of(std::move(tree1), std::move(tree2));
    }

    TYPED_TEST_P(tree_parallel_comparison_test, difference_of_parallel_test) {
        TypeParam tree1(std::move(this -> trees1[this -> index]));
        TypeParam tree2(std::move(this -> trees2[this -> index]));
        this -> index++;
        thread_pool_executor executor;
        difference_of(std::move(tree1), std::move(tree2), executor);
    }


    REGISTER_TYPED_TEST_SUITE_P(tree_parallel_comparison_test,
        union_of_test,
        union_of_parallel_test,
        intersection_of_test,
        intersection_of_parallel_test,
        difference_of_test,
        difference_of_parallel_test
    );
}