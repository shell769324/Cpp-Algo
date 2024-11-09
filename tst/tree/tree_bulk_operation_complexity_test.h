#pragma once
#include "gtest/gtest.h"
#include "tst/utility/common.h"
#include "tst/utility/statistics.h"

namespace {
    using namespace algo;

    template <typename T>
    class tree_bulk_operation_complexity_test : public testing::Test {

    };

    static const int REPEAT = 20;

    TYPED_TEST_SUITE_P(tree_bulk_operation_complexity_test);

    TYPED_TEST_P(tree_bulk_operation_complexity_test, union_of_balance_complexity_test) {
        unsigned total = 2000;
        auto stubs = get_random_stub_vector(total, 0, total * 30);
        TypeParam tree1;
        TypeParam tree2;
        unsigned step = total / REPEAT;
        std::vector<unsigned long> sizes;
        std::vector<unsigned long> durations;
        
        for (unsigned i = 0; i < total / 2 - step; i += step) {
            tree1.insert(stubs.begin() + i, stubs.begin() + i + step);
            tree2.insert(stubs.rbegin() + i, stubs.rbegin() + i + step);
            TypeParam tree1_copy(tree1);
            TypeParam tree2_copy(tree2);
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            union_of(std::move(tree1_copy), std::move(tree2_copy), uid_resolver());
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            sizes.push_back(tree1.size());
            durations.push_back(duration);
        }
        std::cout << "Correlation: " << correlation(sizes, durations) << std::endl;
    }

    TYPED_TEST_P(tree_bulk_operation_complexity_test, union_of_skew_complexity_test) {
        unsigned total = 2000;
        auto stubs = get_random_stub_vector(total, 0, total * 30);
        TypeParam tree1(stubs.begin(), stubs.begin() + stubs.size() / 2);
        TypeParam tree2;
        unsigned step = total / REPEAT;
        std::vector<unsigned long> sizes;
        std::vector<unsigned long> durations;
        
        for (unsigned i = 0; i < total / 2 - step; i += step) {
            tree2.insert(stubs.rbegin() + i, stubs.rbegin() + i + step);
            TypeParam tree1_copy(tree1);
            TypeParam tree2_copy(tree2);
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            union_of(std::move(tree1_copy), std::move(tree2_copy), uid_resolver());
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            sizes.push_back(tree2.size());
            durations.push_back(duration);
        }
        std::vector<double> bigO;
        for (std::size_t i = 0; i < sizes.size(); ++i) {
            bigO.push_back(sizes[i] * std::log((double) tree1.size() / sizes[i] + 1));
        }
        std::cout << "Correlation: " << correlation(bigO, durations) << std::endl;
    }

    TYPED_TEST_P(tree_bulk_operation_complexity_test, intersection_of_balance_complexity_test) {
        unsigned total = 2000;
        auto stubs = get_random_stub_vector(total, 0, total * 30);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam tree1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam tree2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned step = total / REPEAT;
        std::vector<unsigned long> sizes;
        std::vector<unsigned long> durations;
        
        for (unsigned i = 0; i < share_start; i += step) {
            tree1.insert(stubs.begin() + i, stubs.begin() + i + step);
            tree2.insert(stubs.rbegin() + i, stubs.rbegin() + i + step);
            TypeParam tree1_copy(tree1);
            TypeParam tree2_copy(tree2);
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            intersection_of(std::move(tree1_copy), std::move(tree2_copy), uid_resolver());
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            sizes.push_back(tree2.size());
            durations.push_back(duration);
        }
        std::cout << "Correlation: " << correlation(sizes, durations) << std::endl;
    }

    TYPED_TEST_P(tree_bulk_operation_complexity_test, intersection_of_skew_complexity_test) {
        unsigned total = 800;
        auto stubs = get_random_stub_vector(total, 0, total * 30);
        TypeParam tree1(stubs.begin(), stubs.end());
        TypeParam tree2;
        unsigned step = total / REPEAT;
        std::vector<unsigned long> sizes;
        std::vector<unsigned long> durations;
        
        for (unsigned i = 0; i < total; i += step) {
            tree2.insert(stubs.rbegin() + i, stubs.rbegin() + i + step);
            TypeParam tree1_copy(tree1);
            TypeParam tree2_copy(tree2);
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            intersection_of(std::move(tree1_copy), std::move(tree2_copy), uid_resolver());
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            sizes.push_back(tree2.size());
            durations.push_back(duration);
        }
        std::vector<double> bigO;
        for (std::size_t i = 0; i < sizes.size(); ++i) {
            bigO.push_back(sizes[i] * std::log((double) tree1.size() / sizes[i] + 1));
        }
        std::cout << "Correlation: " << correlation(bigO, durations) << std::endl;
    }

    TYPED_TEST_P(tree_bulk_operation_complexity_test, difference_of_balance_complexity_test) {
        unsigned total = 2000;
        auto stubs = get_random_stub_vector(total, 0, total * 30);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam tree1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam tree2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned step = total / REPEAT;
        std::vector<unsigned long> sizes;
        std::vector<unsigned long> durations;
        
        for (unsigned i = 0; i < share_start; i += step) {
            tree1.insert(stubs.begin() + i, stubs.begin() + i + step);
            tree2.insert(stubs.rbegin() + i, stubs.rbegin() + i + step);
            TypeParam tree1_copy(tree1);
            TypeParam tree2_copy(tree2);
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            difference_of(std::move(tree1_copy), std::move(tree2_copy));
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            sizes.push_back(tree1.size());
            durations.push_back(duration);
        }
        std::cout << "Correlation: " << correlation(sizes, durations) << std::endl;
    }

    TYPED_TEST_P(tree_bulk_operation_complexity_test, difference_of_skew_complexity_test) {
        unsigned total = 800;
        auto stubs = get_random_stub_vector(total, 0, total * 30);
        TypeParam tree1(stubs.begin(), stubs.end());
        TypeParam tree2;
        unsigned step = total / REPEAT;
        std::vector<unsigned long> sizes;
        std::vector<unsigned long> durations;
        
        for (unsigned i = 0; i < total; i += step) {
            tree2.insert(stubs.rbegin() + i, stubs.rbegin() + i + step);
            TypeParam tree1_copy(tree1);
            TypeParam tree2_copy(tree2);
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            difference_of(std::move(tree1_copy), std::move(tree2_copy));
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            unsigned long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
            sizes.push_back(tree2.size());
            durations.push_back(duration);
        }
        std::vector<double> bigO;
        for (std::size_t i = 0; i < sizes.size(); ++i) {
            bigO.push_back(sizes[i] * std::log((double) tree1.size() / sizes[i] + 1));
        }
        std::cout << "Correlation: " << correlation(bigO, durations) << std::endl;
    }

    REGISTER_TYPED_TEST_SUITE_P(tree_bulk_operation_complexity_test,
        union_of_balance_complexity_test,
        union_of_skew_complexity_test,
        intersection_of_balance_complexity_test,
        intersection_of_skew_complexity_test,
        difference_of_balance_complexity_test,
        difference_of_skew_complexity_test  
    );
}