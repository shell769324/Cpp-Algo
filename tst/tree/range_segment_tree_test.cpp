#include <algorithm>
#include "gtest/gtest.h"
#include "tree/range_segment_tree.h"
#include "src/vector.h"
#include "tst/utility/constructor_stub.h"
#include <numeric>
#include "tst/utility/common.h"


namespace {
    using namespace algo;
    class range_segment_tree_test : public ::testing::Test {
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int BIG_LIMIT = 1 << 8;
    static const int MEDIUM_LIMIT = 100;
    static const int SMALL_LIMIT = 1 << 4;

    template<typename T, typename Operator, typename RepeatOperator>
    void is_correct(vector<T>& vec, range_segment_tree<T, Operator, RepeatOperator>& tree, Operator op=Operator(), T identity=T()) {
        EXPECT_EQ(vec.size(), tree.size());
        vector<int> prefix_sum(vec.size());

        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = i + 1; j <= vec.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), std::accumulate(vec.begin() + i, vec.begin() + j, identity, op));
            }
        }
    }

    TEST_F(range_segment_tree_test, integer_plus_fill_constructor_test) {
        range_segment_tree<int, std::plus<int>, std::multiplies<int> > tree(SMALL_LIMIT, SMALL_LIMIT);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i + 1; j <= tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), (j - i) * SMALL_LIMIT);
            }
        }
    }

    TEST_F(range_segment_tree_test, integer_plus_range_constructor_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        range_segment_tree<int, std::plus<int>, std::multiplies<int>> tree(vec.begin(), vec.end());
        is_correct(vec, tree);
    }

    TEST_F(range_segment_tree_test, integer_plus_basic_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        range_segment_tree<int, std::plus<int>, std::multiplies<int> > tree(vec.begin(), vec.end());
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            int first = random_number(0, vec.size());
            int last = random_number(first + 1, vec.size() + 1);
            int val = random_number(0, BIG_LIMIT);
            tree.update(first, last, val);
            std::fill(vec.begin() + first, vec.begin() + last, val);
            is_correct(vec, tree);
        }
    }

    TEST_F(range_segment_tree_test, integer_plus_stress_test) {
        vector<int> vec;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            vec.push_back(random_number(0, BIG_LIMIT));
        }
        range_segment_tree<int, std::plus<int>, std::multiplies<int> > tree(vec.begin(), vec.end());
        for (int i = 0; i < BIG_LIMIT; ++i) {
            int first = random_number(0, vec.size());
            int last = random_number(first + 1, vec.size() + 1);
            int val = random_number(0, BIG_LIMIT);
            tree.update(first, last, val);
            std::fill(vec.begin() + first, vec.begin() + last, val);
            if (i % SMALL_LIMIT == 0) {
                is_correct(vec, tree);
            }
        }
    }

    TEST_F(range_segment_tree_test, integer_max_basic_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        auto lambda_max = [](int a, int b) { return std::max(a, b); };
        auto lambda_second = [](std::size_t, int num) { return num; };
        range_segment_tree<int, decltype(lambda_max), decltype(lambda_second)> tree(vec.begin(), vec.end(), lambda_max, lambda_second);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            int first = random_number(0, vec.size());
            int last = random_number(first + 1, vec.size() + 1);
            int val = random_number(0, BIG_LIMIT);
            tree.update(first, last, val);
            std::fill(vec.begin() + first, vec.begin() + last, val);
            is_correct(vec, tree, lambda_max, INT_MIN);
        }
    }

    TEST_F(range_segment_tree_test, integer_max_stress_test) {
        vector<int> vec;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            vec.push_back(random_number(0, BIG_LIMIT));
        }
        auto lambda_max = [](int a, int b) { return std::max(a, b); };
        auto lambda_second = [](std::size_t, int num) { return num; };
        range_segment_tree<int, decltype(lambda_max), decltype(lambda_second)> tree(vec.begin(), vec.end(), lambda_max, lambda_second);
        for (int i = 0; i < BIG_LIMIT; ++i) {
            int first = random_number(0, vec.size());
            int last = random_number(first + 1, vec.size() + 1);
            int val = random_number(0, BIG_LIMIT);
            tree.update(first, last, val);
            std::fill(vec.begin() + first, vec.begin() + last, val);
            if (i % SMALL_LIMIT == 0) {
                is_correct(vec, tree, lambda_max, INT_MIN);
            }
        }
    }

    TEST_F(range_segment_tree_test, integer_min_basic_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        auto lambda_min = [](int a, int b) { return std::min(a, b); };
        auto lambda_second = [](std::size_t, int num) { return num; };
        range_segment_tree<int, decltype(lambda_min), decltype(lambda_second)> tree(vec.begin(), vec.end(), lambda_min, lambda_second);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            int first = random_number(0, vec.size());
            int last = random_number(first + 1, vec.size() + 1);
            int val = random_number(0, BIG_LIMIT);
            tree.update(first, last, val);
            std::fill(vec.begin() + first, vec.begin() + last, val);
            is_correct(vec, tree, lambda_min, INT_MAX);
        }
    }
    

    TEST_F(range_segment_tree_test, integer_positive_leftmost_basic_test) {
        vector<int> vec({-2, 1, 5, -4, -2, 0, 12, -7, 10});
        auto lambda_positive = [](int a) { return a > 0; };
        auto lambda_max = [](int a, int b) { return std::max(a, b); };

        auto lambda_second = [](std::size_t, int num) { return num; };
        range_segment_tree<int, decltype(lambda_max), decltype(lambda_second)> tree(vec.begin(), vec.end(), lambda_max, lambda_second);
        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = i + 1; j <= vec.size(); ++j) {
                auto opt = tree.leftmost(lambda_positive, i, j);
                auto expected = std::find_if(vec.begin() + i, vec.begin() + j, lambda_positive);
                if (opt) {
                    EXPECT_EQ(*opt, expected - vec.begin());
                } else {
                    EXPECT_EQ(expected - vec.begin(), j);
                }
            }
        }
    }

    TEST_F(range_segment_tree_test, integer_positive_leftmost_mixed_test) {
        std::vector<int> vec;
        for (int i = 0; i < BIG_LIMIT; ++i) {
            vec.push_back(random_number(0, BIG_LIMIT));
        }
        auto lambda_positive = [](int a) { return a > 0; };
        auto lambda_max = [](int a, int b) { return std::max(a, b); };

        auto lambda_second = [](std::size_t, int num) { return num; };
        range_segment_tree<int, decltype(lambda_max), decltype(lambda_second)> tree(vec.begin(), vec.end(), lambda_max, lambda_second);
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            int index1 = random_number(0, vec.size());
            int index2 = random_number(index1 + 1, vec.size() + 1);
            int val = random_number(0, BIG_LIMIT);
            tree.update(index1, index2, val);
            std::fill(vec.begin() + index1, vec.begin() + index2, val);
            for (std::size_t j = 0; j < vec.size(); ++j) {
                int last = random_number(j + 1, vec.size() + 1);
                auto opt = tree.leftmost(lambda_positive, j, last);
                auto expected = std::find_if(vec.begin() + j, vec.begin() + last, lambda_positive);
                if (opt) {
                    EXPECT_EQ(*opt, expected - vec.begin());
                } else {
                    EXPECT_EQ(expected - vec.begin(), j);
                }
            }
        }
    }

    TEST_F(range_segment_tree_test, integer_positive_rightmost_basic_test) {
        std::vector<int> vec({-2, 1, 5, -4, -2, 0, 12, -7, 10});
        auto lambda_positive = [](int a) { return a > 0; };
        auto lambda_max = [](int a, int b) { return std::max(a, b); };
        auto lambda_second = [](std::size_t, int num) { return num; };
        range_segment_tree<int, decltype(lambda_max), decltype(lambda_second)> tree(vec.begin(), vec.end(), lambda_max, lambda_second);
        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = i + 1; j <= vec.size(); ++j) {
                auto opt = tree.rightmost(lambda_positive, i, j);
                int has = -1;
                for (int k = j - 1; k >= (int) i; --k) {
                    if (lambda_positive(vec[k])) {
                        has = k;
                        break;
                    }
                }
                if (opt) {
                    EXPECT_EQ(*opt, has);
                } else {
                    EXPECT_EQ(-1, has);
                }
            }
        }
    }
}