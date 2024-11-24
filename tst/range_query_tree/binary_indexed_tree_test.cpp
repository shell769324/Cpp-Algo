#include "gtest/gtest.h"
#include "range_query_tree/binary_indexed_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/utility/common.h"
#include "tst/range_query_tree/invertible_query_update_test.h"
#include "tst/utility/matrix.h"
#include <random>
#include <cmath>

namespace {
    using namespace algo;

    class binary_indexed_tree_test : public ::testing::Test {
    public:
        tracking_allocator<constructor_stub> construct_stub_allocator;
        tracking_allocator<matrix_type> matrix_allocator;
    protected:
        virtual void SetUp() {
            construct_stub_allocator.reset();
            tracking_allocator<matrix_type>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            construct_stub_allocator.check();
            tracking_allocator<matrix_type>::check();
        }
    };

    struct plus_inverse {
        constructor_stub operator()(const constructor_stub& operand, const constructor_stub& sum) const noexcept { 
            return constructor_stub(sum.id - operand.id);
        };
    };

    using constructor_stub_tree = binary_indexed_tree<constructor_stub, std::plus<constructor_stub>, plus_inverse, tracking_allocator<constructor_stub> >;


    static const matrix_type mat1({{0.8, 1.6}, {1, 0.5}});
    static const matrix_type mat2({{1.2, 0.7}, {1.3, 0.3}});
    static constexpr int SMALL_LIMIT = 10;
    static constexpr int LIMIT = 10000;

    TEST_F(binary_indexed_tree_test, matrix_mult_test) {
        auto mat = square_matrix_mult()(mat1, mat2);
        auto mat2_back = square_matrix_multiply_inverse()(mat1, mat);
        EXPECT_TRUE(is_square_matrix_equal(mat2_back, mat2));
    }

    TEST_F(binary_indexed_tree_test, default_fill_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), 0);
            }
        }
        EXPECT_EQ(tree.get_allocator(), this -> construct_stub_allocator);
        EXPECT_EQ(tree.get_identity(), 0);
    }

    TEST_F(binary_indexed_tree_test, fill_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, constructor_stub(1), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j).id, j - i);
            }
        }
        EXPECT_EQ(tree.get_allocator(), this -> construct_stub_allocator);
        EXPECT_EQ(tree.get_identity().id, 0);
    }

    TEST_F(binary_indexed_tree_test, range_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        EXPECT_EQ(tree.size(), stubs.size());
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), std::accumulate(stubs.begin() + i, stubs.begin() + j, constructor_stub(0)));
            }
        }
        EXPECT_EQ(tree.get_allocator(), this -> construct_stub_allocator);
    }

    TEST_F(binary_indexed_tree_test, copy_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, constructor_stub(1));
        EXPECT_EQ(tree.get_identity().id, 0);
        constructor_stub_tree tree_copy(tree);
        for (std::size_t i = 0; i < tree_copy.size(); ++i) {
            for (std::size_t j = i; j < tree_copy.size(); ++j) {
                EXPECT_EQ(tree_copy.query(i, j).id, j - i);
            }
        }
        EXPECT_EQ(tree.get_allocator(), tree_copy.get_allocator());
    }

    TEST_F(binary_indexed_tree_test, move_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, constructor_stub(1), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        std::size_t constructor_invocation = constructor_stub::constructor_invocation_count;
        constructor_stub_tree tree_copy(std::move(tree));
        EXPECT_GE(constructor_invocation + 2, constructor_stub::constructor_invocation_count);
        for (std::size_t i = 0; i < tree_copy.size(); ++i) {
            for (std::size_t j = i; j < tree_copy.size(); ++j) {
                EXPECT_EQ(tree_copy.query(i, j).id, j - i);
            }
        }
        EXPECT_EQ(tree.size(), 0);
        EXPECT_EQ(tree_copy.get_allocator(), this -> construct_stub_allocator);
    }

    TEST_F(binary_indexed_tree_test, assignment_operator_test) {
        constructor_stub_tree tree(SMALL_LIMIT, constructor_stub(1), std::plus<constructor_stub>());
        EXPECT_EQ(tree.get_identity().id, 0);
        constructor_stub_tree tree_copy(SMALL_LIMIT);
        tree_copy = tree;
        for (std::size_t i = 0; i < tree_copy.size(); ++i) {
            for (std::size_t j = i; j < tree_copy.size(); ++j) {
                EXPECT_EQ(tree_copy.query(i, j).id, j - i);
            }
        }
        EXPECT_EQ(tree.get_allocator(), tree_copy.get_allocator());
    }

    TEST_F(binary_indexed_tree_test, move_assignment_operator_test) {
        constructor_stub_tree tree(SMALL_LIMIT, constructor_stub(1), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        constructor_stub_tree tree_copy(SMALL_LIMIT);
        std::size_t constructor_invocation = constructor_stub::constructor_invocation_count;
        tree_copy = std::move(tree);
        EXPECT_GE(constructor_invocation + 2, constructor_stub::constructor_invocation_count);
        for (std::size_t i = 0; i < tree_copy.size(); ++i) {
            for (std::size_t j = i; j < tree_copy.size(); ++j) {
                EXPECT_EQ(tree_copy.query(i, j).id, j - i);
            }
        }
        EXPECT_EQ(tree_copy.get_allocator(), this -> construct_stub_allocator);
    }

    TEST_F(binary_indexed_tree_test, swap_test) {
        std::vector<constructor_stub> stubs1 = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree1(stubs1.begin(), stubs1.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        constructor_stub_tree copy1(tree1);
        std::vector<constructor_stub> stubs2 = get_random_stub_vector(SMALL_LIMIT - 1, -LIMIT, LIMIT);
        tracking_allocator<constructor_stub> local_allocator;
        constructor_stub_tree tree2(stubs2.begin(), stubs2.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(-1), local_allocator);
        constructor_stub_tree copy2(tree2);
        tree1.swap(tree2);
        EXPECT_EQ(tree1, copy2);
        EXPECT_EQ(tree2, copy1);
        EXPECT_EQ(tree1.get_allocator(), copy2.get_allocator());
        EXPECT_EQ(tree2.get_allocator(), copy1.get_allocator());
        EXPECT_EQ(tree1.get_identity(), copy2.get_identity());
        EXPECT_EQ(tree2.get_identity(), copy1.get_identity());
    }

    TEST_F(binary_indexed_tree_test, equality_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree1(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        constructor_stub_tree tree2(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        EXPECT_EQ(tree1, tree2);
    }

    TEST_F(binary_indexed_tree_test, inequality_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree1(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        stubs.front().id--;
        constructor_stub_tree tree2(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        EXPECT_NE(tree1, tree2);
    }

    TEST_F(binary_indexed_tree_test, inequality_identity_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree1(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(1), this -> construct_stub_allocator);
        constructor_stub_tree tree2(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), plus_inverse(), constructor_stub(), this -> construct_stub_allocator);
        EXPECT_NE(tree1, tree2);
    }

    struct type_holder {
        using plus_tree_type = constructor_stub_tree;
        using matrix_mult_tree_type = binary_indexed_tree<matrix_type, square_matrix_mult, square_matrix_multiply_inverse, tracking_allocator<matrix_type> >;
    };

    INSTANTIATE_TYPED_TEST_SUITE_P(binary_indexed_tree, invertible_query_update_test, type_holder);
}