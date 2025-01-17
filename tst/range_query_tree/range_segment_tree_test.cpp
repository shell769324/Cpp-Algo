#include <algorithm>
#include "gtest/gtest.h"
#include "src/range_query_tree/range_segment_tree.h"
#include "src/vector/vector.h"
#include "tst/utility/constructor_stub.h"
#include <numeric>
#include "tst/utility/common.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/utility/matrix.h"
#include "tst/range_query_tree/invertible_query_update_test.h"
#include "tst/range_query_tree/query_update_test.h"


namespace {
    using namespace algo;
    class range_segment_tree_test : public ::testing::Test {
    public:
        tracking_allocator<constructor_stub> constructor_stub_allocator;
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

    struct constructor_stub_multiplies {
        constructor_stub operator()(std::size_t repeat, const constructor_stub& stub) const noexcept {
            return constructor_stub(stub.id * repeat);
        }
    };

    struct matrix_exp {
        matrix_type operator()(std::size_t repeat, const matrix_type& matrix) const noexcept {
            if (repeat == 0) {
                return matrix_type({{1.0, 0}, {0, 1.0}});
            }
            if (repeat == 1) {
                return matrix;
            }
            matrix_type doubled = square_matrix_multiply(matrix, matrix);
            matrix_type res = (*this)(repeat / 2, doubled);
            if (repeat % 2 == 1) {
                return square_matrix_multiply(res, matrix);
            }
            return res;
        }
    };

    struct extrema_repeat {
        constructor_stub operator()(std::size_t, const constructor_stub& stub) const noexcept {
            return stub;
        }
    };

    static constexpr int SMALL_LIMIT = 17;
    static constexpr int MEDIUM_LIMIT = 500;
    static constexpr int GAP = 17;
    static constexpr int BIG_GAP = 29;
    static constexpr int LIMIT = 10000;
    using constructor_stub_tree = range_segment_tree<constructor_stub, std::plus<constructor_stub>, constructor_stub_multiplies, tracking_allocator<constructor_stub> >;
    using constructor_stub_max_tree = range_segment_tree<constructor_stub, constructor_stub_max, extrema_repeat, tracking_allocator<constructor_stub> >;
    using constructor_stub_min_tree = range_segment_tree<constructor_stub, constructor_stub_min, extrema_repeat, tracking_allocator<constructor_stub> >;
    using matrix_mult_tree = range_segment_tree<matrix_type, square_matrix_mult, matrix_exp, tracking_allocator<matrix_type> >;

    template<typename T, typename Operator, typename RepeatOperator, typename Allocator>
    void is_correct(std::vector<T>& vec, range_segment_tree<T, Operator, RepeatOperator, Allocator>& tree, Operator op=Operator(), T identity=T()) {
        EXPECT_EQ(vec.size(), tree.size());

        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = i + 1; j <= vec.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), std::accumulate(vec.begin() + i, vec.begin() + j, identity, op));
            }
        }
    }

    TEST_F(range_segment_tree_test, default_fill_constructor_test) {
        tracking_allocator<int> int_allocator;
        range_segment_tree<int, std::plus<int>, std::multiplies<int>, tracking_allocator<int> > tree(SMALL_LIMIT, std::plus<int>(), std::multiplies<int>(), int_allocator);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i + 1; j <= tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), 0);
            }
        }
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(tree.get_allocator(), int_allocator);
    }

    void one_test(constructor_stub_tree& tree) {
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i + 1; j <= tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j).id, j - i);
            }
        }
    }

    TEST_F(range_segment_tree_test, fill_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        one_test(tree);
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(tree.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(range_segment_tree_test, range_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_tree tree(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        is_correct(stubs, tree, std::plus<constructor_stub>(), constructor_stub(0));
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(tree.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(range_segment_tree_test, copy_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        constructor_stub_tree tree_copy(tree);
        one_test(tree_copy);
        EXPECT_EQ(tree, tree_copy);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(range_segment_tree_test, move_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        std::size_t constructor_invocation = constructor_stub::constructor_invocation_count;
        constructor_stub_tree tree_copy(std::move(tree));
        EXPECT_EQ(constructor_invocation, constructor_stub::constructor_invocation_count);
        one_test(tree_copy);
        EXPECT_EQ(tree.size(), 0);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(range_segment_tree_test, assignment_operator_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        constructor_stub_tree tree_copy(SMALL_LIMIT, 1);
        tree_copy = tree;
        one_test(tree_copy);
        EXPECT_EQ(tree, tree_copy);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(range_segment_tree_test, move_assignemnt_operator_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        constructor_stub_tree tree_copy(SMALL_LIMIT, 1);
        std::size_t constructor_invocation = constructor_stub::constructor_invocation_count;
        tree_copy = std::move(tree);
        EXPECT_EQ(constructor_invocation, constructor_stub::constructor_invocation_count);
        one_test(tree_copy);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(range_segment_tree_test, swap_test) {
        std::vector<constructor_stub> stubs1 = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree1(stubs1.begin(), stubs1.end(), std::plus<constructor_stub>(), constructor_stub_multiplies(), this -> constructor_stub_allocator);
        constructor_stub_tree copy1(tree1);
        std::vector<constructor_stub> stubs2 = get_random_stub_vector(SMALL_LIMIT - 1, -LIMIT, LIMIT);
        tracking_allocator<constructor_stub> local_allocator;
        constructor_stub_tree tree2(stubs2.begin(), stubs2.end(), std::plus<constructor_stub>(), constructor_stub_multiplies(), local_allocator);
        constructor_stub_tree copy2(tree2);
        tree1.swap(tree2);
        EXPECT_EQ(tree1, copy2);
        EXPECT_EQ(tree2, copy1);
        EXPECT_EQ(tree1.get_allocator(), copy2.get_allocator());
        EXPECT_EQ(tree2.__get_subtree_allocator(), copy1.__get_subtree_allocator());
    }

    void is_correct(std::vector<constructor_stub>& vec, constructor_stub_tree& tree, std::size_t gap) {
        EXPECT_EQ(vec.size(), tree.size());
        std::vector<constructor_stub> prefixSums(vec.size());
        std::partial_sum(vec.begin(), vec.end(), prefixSums.begin());
        for (std::size_t d = 1; d <= vec.size(); ++d) {
            for (std::size_t i = 0; i + d <= vec.size(); i += gap) {
                std::size_t j = i + d;
                EXPECT_EQ(tree.query(i, j).id, prefixSums[j - 1].id - (i == 0 ? 0 : prefixSums[i - 1].id));
            }
        }
    }

    TEST_F(range_segment_tree_test, plus_range_update_basic_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < stubs.size(); ++i) {
            int first = random_number(0, stubs.size());
            int last = random_number(first + 1, stubs.size() + 1);
            int val = random_number(-LIMIT, LIMIT);
            tree.update(first, last, val);
            std::fill(stubs.begin() + first, stubs.begin() + last, val);
            is_correct(stubs, tree, 1);
            EXPECT_TRUE(tree.__is_valid());
        }
    }

    TEST_F(range_segment_tree_test, plus_range_update_stress_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < stubs.size(); ++i) {
            int first = random_number(0, stubs.size());
            int last = random_number(first + 1, stubs.size() + 1);
            int val = random_number(-LIMIT, LIMIT);
            tree.update(first, last, val);
            std::fill(stubs.begin() + first, stubs.begin() + last, val);
            if (i % GAP == 0) {
                is_correct(stubs, tree, GAP);
                EXPECT_TRUE(tree.__is_valid());
            }
        }
    }

    template<typename Tree>
    void is_extrema_tree_correct(std::vector<constructor_stub>& stubs, Tree& tree, std::size_t gap) {
        using operator_type = typename Tree::operator_type;
        operator_type op;
        EXPECT_EQ(stubs.size(), tree.size());
        for (std::size_t d = 1; d <= stubs.size(); ++d) {
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

    template<typename Tree>
    void extrema_tree_test() {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT, -LIMIT, LIMIT);
        Tree tree(stubs.begin(), stubs.end());
        for (std::size_t i = 0; i < stubs.size(); ++i) {
            int first = random_number(0, stubs.size());
            int last = random_number(first + 1, stubs.size() + 1);
            int val = random_number(-LIMIT, LIMIT);
            tree.update(first, last, val);
            std::fill(stubs.begin() + first, stubs.begin() + last, val);
            if (i % BIG_GAP == 0) {
                is_extrema_tree_correct(stubs, tree, BIG_GAP);
                EXPECT_TRUE(tree.__is_valid());
            }
        }
    }

    TEST_F(range_segment_tree_test, max_range_update_test) {
        extrema_tree_test<constructor_stub_max_tree>();
    }

    TEST_F(range_segment_tree_test, min_range_update_test) {
        extrema_tree_test<constructor_stub_min_tree>();
    }

    TEST_F(range_segment_tree_test, matrix_mixed_test) {
        matrix_type identity_matrix({{1.0, 0}, {0, 1.0}});
        matrix_mult_tree tree(MEDIUM_LIMIT);
        for (std::size_t i = 0; i < tree.size(); i++) {
            tree.update(i, identity_matrix);
        }
        std::vector<matrix_type> matrices(tree.size(), identity_matrix);
        for (std::size_t i = 0; i < MEDIUM_LIMIT; ++i) {
            int lottery = random_number(0, 2);
            if (lottery) {
                // update
                int first = random_number(0, matrices.size());
                int last = random_number(first + 1, matrices.size() + 1);
                matrix_type mat = get_random_rotation_matrix();
                std::fill(matrices.begin() + first, matrices.begin() + last, mat);
                tree.update(first, last, mat);
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

    struct invertible_type_holder {
        using plus_tree_type = constructor_stub_tree;
        using matrix_mult_tree_type = matrix_mult_tree;
    };

    struct type_holder {
        using max_tree_type = constructor_stub_max_tree;
        using min_tree_type = constructor_stub_min_tree;
    };

    INSTANTIATE_TYPED_TEST_SUITE_P(range_segment_tree, invertible_query_update_test, invertible_type_holder);
    INSTANTIATE_TYPED_TEST_SUITE_P(range_segment_tree, query_update_test, type_holder);
}