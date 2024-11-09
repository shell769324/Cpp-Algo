#include "gtest/gtest.h"
#include "tree/binary_tree_node.h"
#include "tree/binary_tree_iterator.h"
#include "tst/tree/tree_test_util.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/common.h"
#include <iostream>



namespace {
    using namespace algo;
    class binary_tree_iterator_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    static const int SMALL_LIMIT = 1 << 4;

    using node_type = binary_tree_node<constructor_stub>;
    using node_iterator = binary_tree_iterator<constructor_stub>;
    using const_node_iterator = binary_tree_iterator<const constructor_stub>;
    using reverse_node_iterator = binary_tree_iterator<constructor_stub, true>;

    TEST_F(binary_tree_iterator_test, constructor_test) {
        std::unique_ptr<node_type> node_ptr = std::make_unique<node_type>();
        node_iterator it(node_ptr.get());
    }

    TEST_F(binary_tree_iterator_test, access_test) {
        std::unique_ptr<node_type> node_ptr = std::make_unique<node_type>();
        node_iterator it(node_ptr.get());
        EXPECT_EQ((*it).id, node_ptr -> value.id);
        ++(*it).id;
        EXPECT_EQ((*it).id, node_ptr -> value.id);
    }

    TEST_F(binary_tree_iterator_test, const_access_test) {
        std::unique_ptr<node_type> node_ptr = std::make_unique<node_type>();
        const_node_iterator it(node_ptr.get());
        EXPECT_EQ((*it).id, node_ptr -> value.id);
        EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(*std::declval<const_node_iterator&>())> >);
    }

    TEST_F(binary_tree_iterator_test, access_reference_test) {
        std::unique_ptr<node_type> node_ptr = std::make_unique<node_type>();
        node_iterator it(node_ptr.get());
        EXPECT_EQ(it -> id, node_ptr -> value.id);
        ++it -> id;
        EXPECT_EQ(it -> id, node_ptr -> value.id);
    }

    TEST_F(binary_tree_iterator_test, const_access_reference_test) {
        std::unique_ptr<node_type> node_ptr = std::make_unique<node_type>();
        const_node_iterator it(node_ptr.get());
        EXPECT_EQ(it -> id, node_ptr -> value.id);
        EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype((it->id))> >);
    }

    template<typename Iterator>
    void prefix_next_return_test() {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        Iterator it(root.get());
        Iterator next_it = ++it;
        EXPECT_EQ(it -> id, next_it -> id);
    }

    TEST_F(binary_tree_iterator_test, prefix_next_return_test) {
        prefix_next_return_test<node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, reverse_prefix_next_return_test) {
        prefix_next_return_test<reverse_node_iterator>();
    }

    template<typename Iterator>
    void postfix_next_return_test() {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        Iterator it(root.get());
        Iterator next_it = it++;
        if constexpr (std::is_same_v<Iterator, reverse_node_iterator>) {
            EXPECT_EQ(it -> id, next_it -> id - 1);
        } else {
            EXPECT_EQ(it -> id, next_it -> id + 1);
        }
    }

    TEST_F(binary_tree_iterator_test, postfix_next_return_test) {
        postfix_next_return_test<node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, reverse_postfix_next_return_test) {
        postfix_next_return_test<reverse_node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, prefix_forward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_leftmost_descendant();
        node_iterator it(curr);
        for (int i = 0; it != node_iterator(); ++i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_prefix_forward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_rightmost_descendant();
        reverse_node_iterator it(curr);
        for (int i = SMALL_LIMIT - 2; it != reverse_node_iterator(); --i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, postfix_forward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_leftmost_descendant();
        node_iterator it(curr);
        for (int i = 0; it != node_iterator(); ++i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_postfix_forward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_rightmost_descendant();
        reverse_node_iterator it(curr);
        for (int i = SMALL_LIMIT - 2; it != reverse_node_iterator(); --i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    template<typename Iterator>
    void prefix_prev_return_test() {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        Iterator it(root.get());
        Iterator prev_it = --it;
        EXPECT_EQ(it -> id, prev_it -> id);
    }

    TEST_F(binary_tree_iterator_test, prefix_prev_return_test) {
        prefix_prev_return_test<node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, reverse_prefix_prev_return_test) {
        prefix_prev_return_test<reverse_node_iterator>();
    }

    template<typename Iterator>
    void postfix_prev_return_test() {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        Iterator it(root.get());
        Iterator prev_it = it--;
        if constexpr (std::is_same_v<Iterator, reverse_node_iterator>) {
            EXPECT_EQ(it -> id, prev_it -> id + 1);
        } else {
            EXPECT_EQ(it -> id, prev_it -> id - 1);
        }
    }

    TEST_F(binary_tree_iterator_test, postfix_prev_return_test) {
        postfix_prev_return_test<node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, reverse_postfix_prev_return_test) {
        postfix_prev_return_test<reverse_node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, prefix_backward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        for (int i = SMALL_LIMIT - 2; it != node_iterator(); --i, --it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_prefix_backward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_leftmost_descendant();
        reverse_node_iterator it(curr);
        for (int i = 0; it != reverse_node_iterator(); ++i, --it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, postfix_backward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        for (int i = SMALL_LIMIT - 2; it != node_iterator(); --i, it--) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_postfix_backward_range_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_leftmost_descendant();
        reverse_node_iterator it(curr);
        for (int i = 0; it != reverse_node_iterator(); ++i, --it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    template<typename Iterator>
    void equality_test() {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr;
        if constexpr (std::is_same_v<Iterator, reverse_node_iterator>) {
            curr = root -> get_rightmost_descendant();
        } else {
            curr = root -> get_leftmost_descendant();
        }
        Iterator it1(curr);
        Iterator it2(curr);
        EXPECT_EQ(it1, it2);
        ++it2;
        EXPECT_NE(it1, it2);
    }

    TEST_F(binary_tree_iterator_test, equality_test) {
        equality_test<node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, const_equality_test) {
        equality_test<const_node_iterator>();
    }

    TEST_F(binary_tree_iterator_test, const_conversion_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        const_node_iterator const_it(it);
    }

    TEST_F(binary_tree_iterator_test, mixed_const_equality_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        const_node_iterator const_it(it);
        EXPECT_EQ(it, const_it);
        EXPECT_EQ(const_it, it);
        ++it;
        EXPECT_NE(it, const_it);
        EXPECT_NE(const_it, it);
    }
}
