#include <algorithm>
#include "gtest/gtest.h"
#include "tree/segment_tree.h"
#include "src/vector.h"
#include "tst/utility/constructor_stub.h"
#include <numeric>
#include "tst/utility/common.h"
#include <climits>


namespace {
    using namespace algo;
    class segment_tree_test : public ::testing::Test {
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
    static const int SMALL_LIMIT = 1 << 4;

    template<typename T, typename Operator>
    void is_correct(vector<T>& vec, segment_tree<T, Operator>& tree, Operator op=Operator(), T identity=T()) {
        EXPECT_EQ(vec.size(), tree.size());
        vector<int> prefix_sum(vec.size());

        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = i + 1; j <= vec.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), std::accumulate(vec.begin() + i, vec.begin() + j, identity, op));
            }
        }
    }

    TEST_F(segment_tree_test, integer_plus_fill_constructor_test) {
        segment_tree<int, std::plus<int> > tree(SMALL_LIMIT, SMALL_LIMIT);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i + 1; j <= tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), (j - i) * SMALL_LIMIT);
            }
        }
    }

    TEST_F(segment_tree_test, integer_plus_range_constructor_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        segment_tree<int, std::plus<int> > tree(vec.begin(), vec.end());
        is_correct(vec, tree);
    }

    TEST_F(segment_tree_test, integer_plus_basic_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        segment_tree<int, std::plus<int> > tree(vec.begin(), vec.end());
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            int rand = random_number(0, vec.size());
            int val = random_number(0, BIG_LIMIT);
            tree.update(rand, val);
            vec[rand] = val;
            is_correct(vec, tree);
        }
    }

    TEST_F(segment_tree_test, integer_max_basic_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        auto lambda_max = [](int a, int b) { return std::max(a, b); };
        segment_tree<int, decltype(lambda_max) > tree(vec.begin(), vec.end(), lambda_max);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            int rand = random_number(0, vec.size());
            int val = random_number(-SMALL_LIMIT, SMALL_LIMIT);
            tree.update(rand, val);
            vec[rand] = val;
            is_correct(vec, tree, lambda_max, INT_MIN);
        }
    }

    TEST_F(segment_tree_test, integer_min_basic_test) {
        vector<int> vec({1, 5, -4, -2, 0, 12});
        auto lambda_min = [](int a, int b) { return std::min(a, b); };
        segment_tree<int, decltype(lambda_min) > tree(vec.begin(), vec.end(), lambda_min);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            int rand = random_number(0, vec.size());
            int val = random_number(-SMALL_LIMIT, SMALL_LIMIT);
            tree.update(rand, val);
            vec[rand] = val;
            is_correct(vec, tree, lambda_min, INT_MAX);
        }
    }

    TEST_F(segment_tree_test, integer_positive_leftmost_basic_test) {
        vector<int> vec({-2, 1, 5, -4, -2, 0, 12, -7, 10});
        auto lambda_positive = [](int a) { return a > 0; };
        auto lambda_max = [](int a, int b) { return std::max(a, b); };
        segment_tree<int, decltype(lambda_max) > tree(vec.begin(), vec.end(), lambda_max);
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

    TEST_F(segment_tree_test, integer_positive_rightmost_basic_test) {
        std::vector<int> vec({-2, 1, 5, -4, -2, 0, 12, -7, 10});
        auto lambda_positive = [](int a) { return a > 0; };
        auto lambda_max = [](int a, int b) { return std::max(a, b); };
        segment_tree<int, decltype(lambda_max) > tree(vec.begin(), vec.end(), lambda_max);
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