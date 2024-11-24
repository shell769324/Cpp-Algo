#pragma once
#include "gtest/gtest.h"

namespace {

    template <typename T>
    class query_update_test : public testing::Test {
    public:
        static const int LIMIT = 10000;
        static const int MEDIUM_LIMIT = 500;
        static const int SMALL_LIMIT = 18;
        static const int GAP = 17;
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            tracking_allocator<matrix_type>::reset();
            tracking_allocator<constructor_stub>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<matrix_type>::check();
            tracking_allocator<constructor_stub>::check();
        }
    };

    TYPED_TEST_SUITE_P(query_update_test);

    template<typename Tree>
    void test_helper(std::size_t size, std::size_t gap) {
        using Operator = typename Tree::operator_type;
        Operator op;
        std::vector<constructor_stub> stubs = get_random_stub_vector(size);
        Tree tree(stubs.begin(), stubs.end());
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(stubs.size(), tree.size());
        std::vector<constructor_stub> prefixSums(stubs.size());
        std::partial_sum(stubs.begin(), stubs.end(), prefixSums.begin());
        for (std::size_t d = 1; d <= stubs.size(); d += gap) {
            for (std::size_t i = 0; i + d <= stubs.size(); i += gap) {
                std::size_t j = i + d;
                constructor_stub extrema = stubs[i];
                for (std::size_t k = i + 1; k < j; ++k) {
                    extrema = op(extrema, stubs[k]);
                }
                EXPECT_EQ(tree.query(i, j), extrema);
            }
        }
    }

    TYPED_TEST_P(query_update_test, max_basic_test) {
        using max_tree_type = TypeParam::max_tree_type;
        test_helper<max_tree_type>(this -> SMALL_LIMIT, 1);
    }

    TYPED_TEST_P(query_update_test, max_stress_test) {
        using max_tree_type = TypeParam::max_tree_type;
        test_helper<max_tree_type>(this -> MEDIUM_LIMIT, this -> GAP);
    }

    TYPED_TEST_P(query_update_test, min_basic_test) {
        using min_tree_type = TypeParam::min_tree_type;
        test_helper<min_tree_type>(this -> SMALL_LIMIT, 1);
    }

    TYPED_TEST_P(query_update_test, min_stress_test) {
        using min_tree_type = TypeParam::min_tree_type;
        test_helper<min_tree_type>(this -> MEDIUM_LIMIT, this -> GAP);
    }

    TYPED_TEST_P(query_update_test, integer_positive_prefix_search_test) {
        using max_tree_type = TypeParam::max_tree_type;
        std::vector<constructor_stub> stubs = get_random_stub_vector(this -> MEDIUM_LIMIT, -this -> LIMIT, this -> MEDIUM_LIMIT);
        auto lambda_positive = [](constructor_stub a) { return a.id > 0; };
        max_tree_type tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < stubs.size(); ++i) {
            for (std::size_t j = i + 1; j <= stubs.size(); ++j) {
                auto opt = tree.prefix_search(lambda_positive, i, j);
                auto expected = std::find_if(stubs.begin() + i, stubs.begin() + j, lambda_positive);
                if (opt) {
                    EXPECT_EQ(*opt - 1, expected - stubs.begin());
                } else {
                    EXPECT_EQ(expected - stubs.begin(), j);
                }
            }
        }
    }

    TYPED_TEST_P(query_update_test, integer_positive_suffix_search_test) {
        using max_tree_type = TypeParam::max_tree_type;
        std::vector<constructor_stub> stubs = get_random_stub_vector(this -> MEDIUM_LIMIT, -this -> LIMIT, this -> MEDIUM_LIMIT);
        auto lambda_positive = [](constructor_stub a) { return a.id > 0; };
        max_tree_type tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < stubs.size(); ++i) {
            for (std::size_t j = i + 1; j <= stubs.size(); ++j) {
                auto opt = tree.suffix_search(lambda_positive, i, j);
                int has = -1;
                for (int k = j - 1; k >= (int) i; --k) {
                    if (lambda_positive(stubs[k])) {
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

    TYPED_TEST_P(query_update_test, equality_test) {
        using max_tree_type = TypeParam::max_tree_type;
        max_tree_type tree1(this -> SMALL_LIMIT, 1);
        max_tree_type tree2(this -> SMALL_LIMIT, 0);
        for (std::size_t i = 0; i < tree2.size(); i++) {
            tree2.update(i, 1);
        }
        EXPECT_EQ(tree1, tree2);
    }

    TYPED_TEST_P(query_update_test, inequality_test) {
        using max_tree_type = TypeParam::max_tree_type;
        max_tree_type tree1(this -> SMALL_LIMIT, 1);
        max_tree_type tree2(tree1);
        tree2.update(0, 2);
        EXPECT_NE(tree1, tree2);
    }

    REGISTER_TYPED_TEST_SUITE_P(query_update_test, 
        max_basic_test,
        max_stress_test,
        min_basic_test,
        min_stress_test,
        integer_positive_prefix_search_test,
        integer_positive_suffix_search_test,
        equality_test,
        inequality_test
    );
}