#include "gtest/gtest.h"
#include "src/tree/binary_tree_node.h"
#include "src/tree/binary_tree_iterator.h"
#include "tst/tree/tree_test_util.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/common.h"
#include "tst/tree/binary_tree_node_util.h"



namespace {
    using namespace algo;
    class binary_tree_iterator_test : public ::testing::Test {
    protected:
        tracking_allocator<node_type> allocator;
        virtual void SetUp() {
            tracking_allocator<node_type>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<node_type>::check();
        }
    };

    static const int SMALL_LIMIT = 1 << 4;

    using node_type = binary_tree_node<constructor_stub>;
    using node_iterator = binary_tree_iterator<node_type>;
    using const_node_iterator = binary_tree_iterator<binary_tree_node<const constructor_stub>>;
    using reverse_node_iterator = std::reverse_iterator<node_iterator>;

    TEST_F(binary_tree_iterator_test, trait_test) {
        bool correct = std::is_same_v<node_iterator::iterator_category, std::bidirectional_iterator_tag>
            && std::is_same_v<node_iterator::value_type, constructor_stub>
            && std::is_same_v<node_iterator::reference, constructor_stub&>
            && std::is_same_v<node_iterator::pointer, constructor_stub*>
            && std::is_same_v<node_iterator::difference_type, std::ptrdiff_t>;
        EXPECT_TRUE(correct);
    }

    TEST_F(binary_tree_iterator_test, constructor_test) {
        singleton_ptr_type node_ptr = make_singleton();
        EXPECT_EQ(1, allocated<node_type>.load());
        node_iterator it(node_ptr.get());
    }

    TEST_F(binary_tree_iterator_test, access_test) {
        singleton_ptr_type node_ptr = make_singleton();
        node_iterator it(node_ptr.get());
        EXPECT_EQ((*it).id, node_ptr -> value.id);
        ++(*it).id;
        EXPECT_EQ((*it).id, node_ptr -> value.id);
    }

    TEST_F(binary_tree_iterator_test, const_access_test) {
        singleton_ptr_type node_ptr = make_singleton();
        const_node_iterator it(node_ptr.get());
        EXPECT_EQ((*it).id, node_ptr -> value.id);
        EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype(*std::declval<const_node_iterator&>())> >);
    }

    TEST_F(binary_tree_iterator_test, access_reference_test) {
        singleton_ptr_type node_ptr = make_singleton();
        node_iterator it(node_ptr.get());
        EXPECT_EQ(it -> id, node_ptr -> value.id);
        ++it -> id;
        EXPECT_EQ(it -> id, node_ptr -> value.id);
    }

    TEST_F(binary_tree_iterator_test, const_access_reference_test) {
        singleton_ptr_type node_ptr = make_singleton();
        const_node_iterator it(node_ptr.get());
        EXPECT_EQ(it -> id, node_ptr -> value.id);
        EXPECT_TRUE(std::is_const_v<std::remove_reference_t<decltype((it->id))> >);
    }

    template<typename Iterator>
    void prefix_next_return_test() {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
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
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
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
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_leftmost_descendant();
        node_iterator it(curr);
        for (int i = 0; it != node_iterator(); ++i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_prefix_forward_range_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_rightmost_descendant();
        reverse_node_iterator rend(node_iterator(root -> get_leftmost_descendant()));
        curr -> link_right_child(node_type::construct(this -> allocator));
        reverse_node_iterator it(curr -> right_child);
        for (int i = SMALL_LIMIT - 2; it != rend; --i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, postfix_forward_range_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_leftmost_descendant();
        node_iterator it(curr);
        for (int i = 0; it != node_iterator(); ++i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_postfix_forward_range_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_rightmost_descendant();
        reverse_node_iterator rend(node_iterator(root -> get_leftmost_descendant()));
        curr -> link_right_child(node_type::construct(this -> allocator));
        reverse_node_iterator it(curr -> right_child);
        for (int i = SMALL_LIMIT - 2; it != rend; --i, ++it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    template<typename Iterator>
    void prefix_prev_return_test() {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
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
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
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
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        for (int i = SMALL_LIMIT - 2; i >= 1; --i, --it) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_prefix_backward_range_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_leftmost_descendant();
        node_type* rightmost = root -> get_rightmost_descendant();
        rightmost -> link_right_child(node_type::construct(this -> allocator));
        reverse_node_iterator rbegin(node_iterator(rightmost -> right_child));
        reverse_node_iterator it(node_iterator{curr});
        --it;
        for (int i = 0; ; ++i, --it) {
            EXPECT_EQ(it -> id, i);
            if (it == rbegin) {
                break;
            }
        }
    }

    TEST_F(binary_tree_iterator_test, postfix_backward_range_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        for (int i = SMALL_LIMIT - 2; i >= 1; --i, it--) {
            EXPECT_EQ(it -> id, i);
        }
    }

    TEST_F(binary_tree_iterator_test, reverse_postfix_backward_range_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_leftmost_descendant();
        node_type* rightmost = root -> get_rightmost_descendant();
        rightmost -> link_right_child(node_type::construct(this -> allocator));
        reverse_node_iterator rbegin(node_iterator(rightmost -> right_child));
        reverse_node_iterator it(node_iterator{curr});
        it--;
        for (int i = 0; ; ++i, it--) {
            EXPECT_EQ(it -> id, i);
            if (it == rbegin) {
                break;
            }
        }
    }

    template<typename Iterator>
    void equality_test() {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
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
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
        node_type* curr = root -> get_rightmost_descendant();
        node_iterator it(curr);
        const_node_iterator const_it(it);
    }

    TEST_F(binary_tree_iterator_test, mixed_const_equality_test) {
        tree_ptr_type root(create_perfectly_balance_tree(SMALL_LIMIT - 1, 0));
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
