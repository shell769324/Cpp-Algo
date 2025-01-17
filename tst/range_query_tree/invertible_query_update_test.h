#pragma once
#include <concepts>
#include "gtest/gtest.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/common.h"
#include "tst/utility/matrix.h"
#include "tst/utility/tracking_allocator.h"

namespace {
    using namespace algo;

    template <typename T>
    class invertible_query_update_test : public testing::Test {
        public:
            static constexpr int LIMIT = 10000;
            static constexpr int MEDIUM_LIMIT = 500;
            static constexpr int SMALL_LIMIT = 18;
            static constexpr int GAP = 100;
            static constexpr int MATRIX_TEST_SIZE = 100;
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

    TYPED_TEST_SUITE_P(invertible_query_update_test);

    template<typename Tree>
    void is_correct(std::vector<constructor_stub>& vec, Tree& tree, std::size_t gap) {
        EXPECT_EQ(vec.size(), tree.size());
        std::vector<constructor_stub> prefixSums(vec.size());
        std::partial_sum(vec.begin(), vec.end(), prefixSums.begin());
        for (std::size_t d = 1; d <= vec.size(); ++d) {
            for (std::size_t i = 0; i + d <= vec.size(); i += gap) {
                std::size_t j = i + d;
                EXPECT_EQ(tree.query(i, j), prefixSums[j - 1].id - (i == 0 ? 0 : prefixSums[i - 1].id));
            }
        }
    }

    TYPED_TEST_P(invertible_query_update_test, plus_query_basic_test) {
        using plus_tree_type = TypeParam::plus_tree_type;
        std::vector<constructor_stub> stubs = get_random_stub_vector(this -> SMALL_LIMIT);
        plus_tree_type tree(stubs.begin(), stubs.end());
        is_correct(stubs, tree, 1);
        EXPECT_TRUE(tree.__is_valid());
    }

    TYPED_TEST_P(invertible_query_update_test, plus_query_stress_test) {
        using plus_tree_type = TypeParam::plus_tree_type;
        std::vector<constructor_stub> stubs = get_random_stub_vector(this -> MEDIUM_LIMIT);
        plus_tree_type tree(stubs.begin(), stubs.end());
        is_correct(stubs, tree, this -> GAP);
        EXPECT_TRUE(tree.__is_valid());
    }

    TYPED_TEST_P(invertible_query_update_test, plus_mixed_basic_test) {
        using plus_tree_type = TypeParam::plus_tree_type;
        std::vector<constructor_stub> stubs = get_random_stub_vector(this -> SMALL_LIMIT, -this -> MEDIUM_LIMIT, this -> MEDIUM_LIMIT);
        plus_tree_type tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < this -> SMALL_LIMIT; ++i) {
            int rand = random_number(0, stubs.size());
            int val = random_number(-this -> LIMIT, this -> LIMIT);
            tree.update(rand, val);
            stubs[rand] = val;
            is_correct(stubs, tree, 1);
            EXPECT_TRUE(tree.__is_valid());
        }
    }

    TYPED_TEST_P(invertible_query_update_test, plus_mixed_stress_test) {
        using plus_tree_type = TypeParam::plus_tree_type;
        std::vector<constructor_stub> stubs = get_random_stub_vector(this -> MEDIUM_LIMIT, -this -> LIMIT, this -> LIMIT);
        plus_tree_type tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < this -> MEDIUM_LIMIT; ++i) {
            int rand = random_number(0, stubs.size());
            int val = random_number(-this -> LIMIT, this -> LIMIT);
            tree.update(rand, val);
            stubs[rand] = val;
            if (this -> MEDIUM_LIMIT % this -> GAP == 0) {
                is_correct(stubs, tree, this -> GAP);
                EXPECT_TRUE(tree.__is_valid());
            }
        }
    }

    TYPED_TEST_P(invertible_query_update_test, identity_matrix_test) {
        using matrix_mult_tree_type = TypeParam::matrix_mult_tree_type;
        matrix_type identity_matrix({{1.0, 0}, {0, 1.0}});
        std::vector<matrix_type> matrices(this -> SMALL_LIMIT);
        matrix_mult_tree_type tree(matrices.begin(), matrices.end());
        for (std::size_t d = 1; d <= matrices.size(); ++d) {
            for (std::size_t i = 0; i + d <= matrices.size(); ++i) {
                std::size_t j = i + d;
                EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), identity_matrix));
            }
        }
        EXPECT_TRUE(tree.__is_valid());
    }

    TYPED_TEST_P(invertible_query_update_test, matrix_query_stress_test) {
        using matrix_mult_tree_type = TypeParam::matrix_mult_tree_type;
        matrix_type identity_matrix({{1.0, 0}, {0, 1.0}});
        std::vector<matrix_type> matrices;
        for (std::size_t i = 0; i < this -> MATRIX_TEST_SIZE; ++i) {
            matrices.push_back(get_random_rotation_matrix());
        }
        matrix_mult_tree_type tree(matrices.begin(), matrices.end());
        for (std::size_t d = 1; d <= matrices.size(); ++d) {
            for (std::size_t i = 0; i + d <= matrices.size(); ++i) {
                std::size_t j = i + d;
                matrix_type expected(identity_matrix);
                for (std::size_t k = i; k < j; ++k) {
                    expected = square_matrix_multiply(expected, matrices[k]);
                }
                EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), expected));
            }
        }
        EXPECT_TRUE(tree.__is_valid());
    }

    TYPED_TEST_P(invertible_query_update_test, matrix_mixed_test) {
        using matrix_mult_tree_type = TypeParam::matrix_mult_tree_type;
        matrix_type identity_matrix({{1.0, 0}, {0, 1.0}});
        matrix_mult_tree_type tree(this -> MEDIUM_LIMIT);
        for (std::size_t i = 0; i < tree.size(); i++) {
            tree.update(i, identity_matrix);
        }
        std::vector<matrix_type> matrices(tree.size(), identity_matrix);
        for (std::size_t i = 0; i < this -> MEDIUM_LIMIT; ++i) {
            int lottery = random_number(0, 2);
            if (lottery) {
                // update
                int idx = random_number(0, matrices.size());
                matrix_type mat = get_random_rotation_matrix();
                matrices[idx] = mat;
                tree.update(idx, mat);
            } else {
                // query
                int idx1 = random_number(0, matrices.size());
                int idx2 = random_number(0, matrices.size());
                if (idx1 > idx2) {
                    std::swap(idx1, idx2);
                }
                if (idx1 == idx2) {
                    continue;
                }
                matrix_type expected(identity_matrix);
                for (int j = idx1; j < idx2; ++j) {
                    expected = square_matrix_multiply(expected, matrices[j]);
                }
                EXPECT_TRUE(is_square_matrix_equal(tree.query(idx1, idx2), expected));
            }
        }
    }

    REGISTER_TYPED_TEST_SUITE_P(invertible_query_update_test, 
        plus_query_basic_test,
        plus_query_stress_test,
        plus_mixed_basic_test,
        plus_mixed_stress_test,
        identity_matrix_test,
        matrix_query_stress_test,
        matrix_mixed_test
    );
}