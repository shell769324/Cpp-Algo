#include <algorithm>
#include "gtest/gtest.h"
#include "range_query_tree/segment_tree.h"
#include "src/vector.h"
#include "tst/utility/constructor_stub.h"
#include <numeric>
#include "tst/utility/common.h"
#include "tst/utility/matrix.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/range_query_tree/invertible_query_update_test.h"
#include "tst/range_query_tree/query_update_test.h"
#include <climits>


namespace {
    using namespace algo;
    class segment_tree_test : public ::testing::Test {
    public:
        tracking_allocator<constructor_stub> constructor_stub_allocator;
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            constructor_stub_allocator.reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            constructor_stub_allocator.check();
        }
    };

    static constexpr int SMALL_LIMIT = 10;
    static constexpr int LIMIT = 10000;
    using constructor_stub_tree = segment_tree<constructor_stub, std::plus<constructor_stub>, tracking_allocator<constructor_stub> >;
    using matrix_mult_tree = segment_tree<matrix_type, square_matrix_mult, tracking_allocator<matrix_type> >;

    template<typename T, typename Operator, typename Allocator>
    void is_correct(std::vector<T>& vec, segment_tree<T, Operator, Allocator>& tree, Operator op=Operator(), T identity=T()) {
        EXPECT_EQ(vec.size(), tree.size());

        for (std::size_t i = 0; i < vec.size(); ++i) {
            for (std::size_t j = i + 1; j <= vec.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), std::accumulate(vec.begin() + i, vec.begin() + j, identity, op));
            }
        }
    }

    TEST_F(segment_tree_test, default_fill_constructor_test) {
        tracking_allocator<int> int_allocator;
        segment_tree<int, std::plus<int>, tracking_allocator<int> > tree(SMALL_LIMIT, std::plus<int>(), int_allocator);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i + 1; j <= tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), 0);
            }
        }
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(tree.get_allocator(), int_allocator);
    }

    void one_test(const constructor_stub_tree& tree) {
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i + 1; j <= tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j).id, j - i);
            }
        }
    }

    TEST_F(segment_tree_test, fill_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), this -> constructor_stub_allocator);
        one_test(tree);
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(tree.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(segment_tree_test, range_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_tree tree(stubs.begin(), stubs.end(), std::plus<constructor_stub>(), this -> constructor_stub_allocator);
        is_correct(stubs, tree, std::plus<constructor_stub>(), constructor_stub(0));
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(tree.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(segment_tree_test, copy_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), this -> constructor_stub_allocator);
        constructor_stub_tree tree_copy(tree);
        one_test(tree_copy);
        EXPECT_EQ(tree, tree_copy);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(segment_tree_test, move_constructor_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), this -> constructor_stub_allocator);
        std::size_t constructor_invocation = constructor_stub::constructor_invocation_count;
        constructor_stub_tree tree_copy(std::move(tree));
        EXPECT_EQ(constructor_invocation, constructor_stub::constructor_invocation_count);
        one_test(tree_copy);
        EXPECT_EQ(tree.size(), 0);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(segment_tree_test, assignment_operator_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), this -> constructor_stub_allocator);
        constructor_stub_tree tree_copy(SMALL_LIMIT, 1);
        tree_copy = tree;
        one_test(tree_copy);
        EXPECT_EQ(tree, tree_copy);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(segment_tree_test, move_assignemnt_operator_test) {
        constructor_stub_tree tree(SMALL_LIMIT, 1, std::plus<constructor_stub>(), this -> constructor_stub_allocator);
        constructor_stub_tree tree_copy(SMALL_LIMIT, 1);
        std::size_t constructor_invocation = constructor_stub::constructor_invocation_count;
        tree_copy = std::move(tree);
        EXPECT_EQ(constructor_invocation, constructor_stub::constructor_invocation_count);
        one_test(tree_copy);
        EXPECT_TRUE(tree_copy.__is_valid());
        EXPECT_EQ(tree_copy.get_allocator(), this -> constructor_stub_allocator);
    }

    TEST_F(segment_tree_test, swap_test) {
        std::vector<constructor_stub> stubs1 = get_random_stub_vector(SMALL_LIMIT, -LIMIT, LIMIT);
        constructor_stub_tree tree1(stubs1.begin(), stubs1.end(), std::plus<constructor_stub>(),  this -> constructor_stub_allocator);
        constructor_stub_tree copy1(tree1);
        std::vector<constructor_stub> stubs2 = get_random_stub_vector(SMALL_LIMIT - 1, -LIMIT, LIMIT);
        tracking_allocator<constructor_stub> local_allocator;
        constructor_stub_tree tree2(stubs2.begin(), stubs2.end(), std::plus<constructor_stub>(), local_allocator);
        constructor_stub_tree copy2(tree2);
        tree1.swap(tree2);
        EXPECT_EQ(tree1, copy2);
        EXPECT_EQ(tree2, copy1);
        EXPECT_EQ(tree1.get_allocator(), copy2.get_allocator());
        EXPECT_EQ(tree2.__get_subtree_allocator(), copy1.__get_subtree_allocator());
    }

    struct invertible_type_holder {
        using plus_tree_type = constructor_stub_tree;
        using matrix_mult_tree_type = segment_tree<matrix_type, square_matrix_mult, tracking_allocator<matrix_type> >;
    };

    struct type_holder {
        using max_tree_type = segment_tree<constructor_stub, constructor_stub_max, tracking_allocator<constructor_stub> >;
        using min_tree_type = segment_tree<constructor_stub, constructor_stub_min, tracking_allocator<constructor_stub> >;
    };

    INSTANTIATE_TYPED_TEST_SUITE_P(segment_tree, invertible_query_update_test, invertible_type_holder);
    INSTANTIATE_TYPED_TEST_SUITE_P(segment_tree, query_update_test, type_holder);
}