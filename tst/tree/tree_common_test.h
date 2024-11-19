#pragma once
#include "gtest/gtest.h"
#include <concepts>
#include "tst/utility/tracking_allocator.h"

namespace {
    using namespace algo;

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SPECIAL_VALUE2 = 0xdeadbabe;
    static const int LIMIT = 10000;
    static const int MEDIUM_LIMIT = 1000;
    static const int SMALL_LIMIT = 10;

    template <typename T>
    class tree_common_test : public testing::Test {
    public:
        tracking_allocator<constructor_stub> allocator;

    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            tracking_allocator<typename T::node_type>::reset();
            tracking_allocator<typename T::value_type>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<typename T::node_type>::check();
            tracking_allocator<typename T::value_type>::check();
        }

    public:
        void is_equal_tree_test(const T& tree1, const T& tree2) {
            EXPECT_EQ(tree1.size(), tree2.size());
            for (const constructor_stub& val : tree1) {
                EXPECT_NE(tree2.find(val), tree2.cend());
            }
        }
    };
    

    TYPED_TEST_SUITE_P(tree_common_test);

    // Constructor and assignment operator tests
    TYPED_TEST_P(tree_common_test, default_constructor_test) {
        TypeParam tree;
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(1, allocated<typename TypeParam::node_type>);
    }

    TYPED_TEST_P(tree_common_test, comp_allocator_constructor_test) {
        constructor_stub_comparator comp;
        TypeParam tree(comp, this -> allocator);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_TRUE(tree.__is_valid());
        EXPECT_EQ(comp, tree.key_comp());
        EXPECT_EQ(this -> allocator.id, tree.__get_node_allocator().id);
        EXPECT_EQ(1, allocated<typename TypeParam::node_type>);
        EXPECT_EQ(0, deallocated<typename TypeParam::node_type>);
        tree.emplace(1);
        EXPECT_EQ(2, allocated<typename TypeParam::node_type>);
        tree.erase(1);
        EXPECT_EQ(1, deallocated<typename TypeParam::node_type>);
        EXPECT_EQ(2, allocated<typename TypeParam::node_type>);
    }

    TYPED_TEST_P(tree_common_test, range_constructor_test) {
        constructor_stub_comparator comp;
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend(), comp, this -> allocator);
        EXPECT_EQ(1 + stubs.size(), allocated<typename TypeParam::node_type>);
        EXPECT_EQ(comp, tree.key_comp());
        EXPECT_EQ(this -> allocator.id, tree.__get_node_allocator().id);
        for (auto& val : stubs) {
            EXPECT_NE(tree.find(val), tree.cend());
        }
    }

    TYPED_TEST_P(tree_common_test, copy_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend());
        TypeParam tree_copy(tree);
        this -> is_equal_tree_test(tree, tree_copy);
        EXPECT_EQ(tree.key_comp(), tree_copy.key_comp());
        EXPECT_EQ(tree.__get_node_allocator(), tree_copy.__get_node_allocator());
    }

    TYPED_TEST_P(tree_common_test, allocator_copy_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend());
        TypeParam tree_copy(tree, this -> allocator);
        EXPECT_TRUE(tree_copy.__is_valid());
        this -> is_equal_tree_test(tree, tree_copy);
        EXPECT_EQ(tree.key_comp(), tree_copy.key_comp());
        EXPECT_EQ(this -> allocator.id, tree_copy.__get_node_allocator().id);
    }

    TYPED_TEST_P(tree_common_test, move_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend());
        TypeParam tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        TypeParam tree_move(std::move(tree));
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        this -> is_equal_tree_test(tree_move, tree_copy);
        EXPECT_EQ(tree_copy.key_comp(), tree_move.key_comp());
        EXPECT_EQ(tree_copy.__get_node_allocator(), tree_move.__get_node_allocator());
    }

    TYPED_TEST_P(tree_common_test, allocator_move_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend());
        TypeParam tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        TypeParam tree_move(std::move(tree), this -> allocator);
        EXPECT_TRUE(tree_move.__is_valid());
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        this -> is_equal_tree_test(tree_move, tree_copy);
        EXPECT_EQ(tree_copy.key_comp(), tree_move.key_comp());
        EXPECT_EQ(this -> allocator.id, tree_move.__get_node_allocator().id);
    }

    TYPED_TEST_P(tree_common_test, copy_assign_operator_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend());
        TypeParam tree_copy;
        tree_copy = tree;
        this -> is_equal_tree_test(tree, tree_copy);
        EXPECT_EQ(tree.key_comp(), tree_copy.key_comp());
        EXPECT_EQ(tree.__get_node_allocator(), tree_copy.__get_node_allocator());
    }

    TYPED_TEST_P(tree_common_test, move_assign_operator_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree(stubs.cbegin(), stubs.cend());
        TypeParam tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        TypeParam tree_move;
        tree_move = std::move(tree);
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        this -> is_equal_tree_test(tree_move, tree_copy);
        EXPECT_EQ(tree_copy.key_comp(), tree_move.key_comp());
        EXPECT_EQ(tree_copy.__get_node_allocator(), tree_move.__get_node_allocator());
    }

    TYPED_TEST_P(tree_common_test, swap_test) {
        std::vector<constructor_stub> stubs1 = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree1(stubs1.cbegin(), stubs1.cend());
        TypeParam tree_copy1(tree1);
        std::vector<constructor_stub> stubs2 = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree2(stubs2.cbegin(), stubs2.cend());
        TypeParam tree_copy2(tree2);
        std::swap(tree1, tree2);
        this -> is_equal_tree_test(tree2, tree_copy1);
        this -> is_equal_tree_test(tree1, tree_copy2);
        EXPECT_EQ(tree2.key_comp(), tree_copy1.key_comp());
        EXPECT_EQ(tree2.__get_node_allocator(), tree_copy1.__get_node_allocator());
        EXPECT_EQ(tree1.key_comp(), tree_copy2.key_comp());
        EXPECT_EQ(tree1.__get_node_allocator(), tree_copy2.__get_node_allocator());
    }

    TYPED_TEST_P(tree_common_test, equality_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree1(stubs.cbegin(), stubs.cend());
        TypeParam tree2(stubs.cbegin(), stubs.cend());
        EXPECT_EQ(tree1, tree2);
    }

    TYPED_TEST_P(tree_common_test, inequality_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree1(stubs.cbegin(), stubs.cend());
        TypeParam tree2(stubs.cbegin(), stubs.cend() - 1);
        EXPECT_NE(tree1, tree2);
    }

    TYPED_TEST_P(tree_common_test, three_way_comparison_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree1(stubs.cbegin(), stubs.cend());
        stubs.back().id++;
        TypeParam tree2(stubs.cbegin(), stubs.cend());
        EXPECT_LE(tree1, tree2);
        EXPECT_LT(tree1, tree2);
        EXPECT_GT(tree2, tree1);
        EXPECT_GE(tree2, tree1);
    }

    TYPED_TEST_P(tree_common_test, three_way_comparison_length_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam tree1(stubs.cbegin(), stubs.cend());
        tree1.erase(std::prev(tree1.end()));
        TypeParam tree2(stubs.cbegin(), stubs.cend());
        EXPECT_LE(tree1, tree2);
        EXPECT_LT(tree1, tree2);
        EXPECT_GT(tree2, tree1);
        EXPECT_GE(tree2, tree1);
    }

    REGISTER_TYPED_TEST_SUITE_P(tree_common_test,
        default_constructor_test,
        comp_allocator_constructor_test,
        range_constructor_test,
        copy_constructor_test,
        allocator_copy_constructor_test,
        move_constructor_test,
        allocator_move_constructor_test,
        copy_assign_operator_test,
        move_assign_operator_test,
        swap_test,
        equality_test,
        inequality_test,
        three_way_comparison_test,
        three_way_comparison_length_test
    );
}