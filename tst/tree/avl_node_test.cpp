#include "gtest/gtest.h"
#include "src/tree/avl_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/tree/tree_test_util.h"
#include "tst/utility/tracking_allocator.h"


namespace {
    using namespace algo;
    using node_type = avl_node<constructor_stub>;

    class avl_node_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            tracking_allocator<node_type>::reset();
            tracking_allocator<constructor_stub>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<node_type>::check();
            tracking_allocator<constructor_stub>::check();
        }
    };

    static const unsigned char SPECIAL_VALUE = 0x3e;
    static const unsigned char SPECIAL_VALUE2 = 0x2a;
    static const int SMALL_LIMIT = 1 << 4;
    static tracking_allocator<node_type> allocator;

    struct singleton_cleaner {
        
        void operator()(node_type* node) {
            node -> destroy(allocator);
        }
    };

    struct tree_cleaner {
        void operator()(node_type* node) {
            node -> deep_destroy(allocator);
        }
    };

    using singleton_ptr_type = std::unique_ptr<node_type, singleton_cleaner>;
    using tree_ptr_type = std::unique_ptr<node_type, tree_cleaner>;

    template <typename... Args>
    singleton_ptr_type make_singleton(Args&&... args) {
        node_type* node = node_type::construct(allocator, std::forward<Args>(args)...);
        return singleton_ptr_type(node);
    }

    template <typename... Args>
    tree_ptr_type make_tree(Args&&... args) {
        node_type* node = node_type::construct(allocator, std::forward<Args>(args)...);
        return tree_ptr_type(node);
    }

    node_type* make_node(signed char height) {
        node_type* node = node_type::construct(allocator);
        node -> height = height;
        return node;
    }

    TEST_F(avl_node_test, construct_sentinel_test) {
        node_type* node = node_type::construct_sentinel(allocator);
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        is_missing_parent_children(node);
        EXPECT_EQ(allocated<node_type>, 1);
        EXPECT_EQ(constructed<constructor_stub>, 0);
        allocator.deallocate(node, 1);
    }

    TEST_F(avl_node_test, construct_default_test) {
        singleton_ptr_type node = make_singleton();
        EXPECT_EQ(node -> height, 1);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(allocated<node_type>, 1);
    }

    TEST_F(avl_node_test, construct_perfect_forward_args_test) {
        singleton_ptr_type node = make_singleton(SPECIAL_VALUE);
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(node -> value.id, SPECIAL_VALUE);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(allocated<node_type>, 1);
    }

    TEST_F(avl_node_test, construct_perfect_forward_lvalue_test) {
        constructor_stub stub(SPECIAL_VALUE);
        singleton_ptr_type node = make_singleton(stub);
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(node -> value, stub);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(allocated<node_type>, 1);
    }

    TEST_F(avl_node_test, construct_perfect_forward_rvalue_test) {
        singleton_ptr_type node = make_singleton(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(node -> value.id, SPECIAL_VALUE);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(allocated<node_type>, 1);
    }

    TEST_F(avl_node_test, clone_test) {
        singleton_ptr_type node = make_singleton(constructor_stub(SPECIAL_VALUE));
        node -> height = SPECIAL_VALUE;
        singleton_ptr_type node_copy(node -> clone(allocator));
        EXPECT_EQ(node -> height, node_copy -> height);
        EXPECT_EQ(node -> value, node_copy -> value);
        is_missing_parent_children(node_copy.get());
        EXPECT_EQ(allocated<node_type>, 2);
    }

    TEST_F(avl_node_test, compute_balance_factor_test) {
        tree_ptr_type node = make_tree(constructor_stub(SPECIAL_VALUE));
        node -> link_left_child(make_node(SPECIAL_VALUE));
        node -> link_right_child(make_node(SPECIAL_VALUE2));
        EXPECT_EQ((int) node -> compute_balance_factor(), (int) SPECIAL_VALUE - SPECIAL_VALUE2);
    }

    TEST_F(avl_node_test, update_height_test) {
        tree_ptr_type node = make_tree(constructor_stub(SPECIAL_VALUE));
        node -> link_left_child(make_node(SPECIAL_VALUE));
        node -> link_right_child(make_node(SPECIAL_VALUE2));
        node -> update_height();
        EXPECT_EQ(node -> height, 1 + std::max(SPECIAL_VALUE, SPECIAL_VALUE2));
    }

    void rotate_left_test(bool has_C) {
        /*
         *       P                P
         *      /                /
         *     B               D
         *    / \      ->     / \
         *   A   D           B   E
         *      / \         / \
         *    [C]  E       A  [C]
         */
        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(3);
        node_type* C;
        node_type* D = make_node(2);
        node_type* E = node_type::construct(allocator);
        tree_ptr_type P(make_node(4));

        B -> link_left_child(A);
        B -> link_right_child(D);
        if (has_C) {
            C = node_type::construct(allocator);
            D -> link_left_child(C);
        }
        D -> link_right_child(E);
        P -> link_left_child(B);
        int old_constructor_invocation_count = constructor_stub::constructor_invocation_count;
        B -> rotate_left<true>();
        EXPECT_EQ(old_constructor_invocation_count, constructor_stub::constructor_invocation_count);
        EXPECT_TRUE(A -> is_leaf());
        if (has_C) {
            EXPECT_TRUE(C -> is_leaf());
            EXPECT_EQ(B -> right_child, C);
        } else {
            EXPECT_EQ(B -> right_child, nullptr);
        }
        EXPECT_TRUE(E -> is_leaf());
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> height, 2);
        EXPECT_EQ(D -> left_child, B);
        EXPECT_EQ(D -> height, 3);
        EXPECT_EQ(D -> right_child, E);
        EXPECT_EQ(P -> left_child, D);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rotate_left_full_house_test) {
        rotate_left_test(true);
    }

    TEST_F(avl_node_test, rotate_left_missing_right_left_child_test) {
        rotate_left_test(false);
    }

    /*
     *      P               P
     *       \               \
     *        D               B
     *       / \      ->     / \
     *      B   E           A   D
     *     / \                 / \
     *    A  [C]             [C]  E
     */
    void rotate_right_test(bool has_C) {
        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(2);
        node_type* C;
        node_type* D = make_node(3);
        node_type* E = node_type::construct(allocator);
        tree_ptr_type P(make_node(4));

        B -> link_left_child(A);
        if (has_C) {
            C = node_type::construct(allocator);
            B -> link_right_child(C);
        }
        D -> link_left_child(B);
        D -> link_right_child(E);
        P -> link_right_child(D);
        int old_constructor_invocation_count = constructor_stub::constructor_invocation_count;
        D -> rotate_right<true>();
        EXPECT_EQ(old_constructor_invocation_count, constructor_stub::constructor_invocation_count);
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_TRUE(E -> is_leaf());
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> height, 3);
        if (has_C) {
            EXPECT_TRUE(C -> is_leaf());
            EXPECT_EQ(D -> left_child, C);
        } else {
            EXPECT_EQ(D -> left_child, nullptr);
        }
        EXPECT_EQ(D -> height, 2);
        EXPECT_EQ(D -> right_child, E);
        EXPECT_EQ(P -> right_child, B);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rotate_right_full_house_test) {
        rotate_right_test(true);
    }

    TEST_F(avl_node_test, rotate_right_missing_left_right_child_test) {
        rotate_right_test(false);
    }

    void rotate_stress_test(bool has_C) {
        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(2);
        node_type* C;
        node_type* D = make_node(3);
        node_type* E = node_type::construct(allocator);
        tree_ptr_type P(make_node(4));

        B -> link_left_child(A);
        if (has_C) {
            C = node_type::construct(allocator);
            B -> link_right_child(C);
        }
        D -> link_left_child(B);
        D -> link_right_child(E);
        P -> link_right_child(D);
        int old_constructor_invocation_count = constructor_stub::constructor_invocation_count;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            D -> rotate_right<true>();
            B -> rotate_left<true>();
        }
        D -> rotate_right<true>();
        EXPECT_EQ(old_constructor_invocation_count, constructor_stub::constructor_invocation_count);
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_TRUE(E -> is_leaf());
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> height, 3);
        if (has_C) {
            EXPECT_TRUE(C -> is_leaf());
            EXPECT_EQ(D -> left_child, C);
        } else {
            EXPECT_EQ(D -> left_child, nullptr);
        }
        EXPECT_EQ(D -> height, 2);
        EXPECT_EQ(D -> right_child, E);
        EXPECT_EQ(P -> right_child, B);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rotate_full_house_stress_test) {
        rotate_stress_test(true);
    }

    TEST_F(avl_node_test, rotate_missing_child_stress_test) {
        rotate_stress_test(false);
    }

    TEST_F(avl_node_test, rebalance_left_left_basic_test) {
        /*
         *          P
         *         /           P
         *        C           /
         *       /    ->     B
         *      B           / \
         *     /           A   C
         *    A
         */

        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(2);
        node_type* C = make_node(3);
        tree_ptr_type P(make_node(4));

        B -> link_left_child(A);
        C -> link_left_child(B);
        P -> link_left_child(C);
        C -> rebalance_left<true>();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> left_child, B);
        // Parent height shouldn't change
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rebalance_left_left_complex_test) {
        /*
         *             P   
         *            /              P
         *           E              /
         *          / \            C
         *         C   F    ->    / \
         *        / \            B   E
         *       B   D          /   / \
         *      /              A   D   F
         *     A
         */

        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(2);
        node_type* C = make_node(3);
        node_type* D = node_type::construct(allocator);
        node_type* E = make_node(4);
        node_type* F = node_type::construct(allocator);
        tree_ptr_type P(make_node(5));

        B -> link_left_child(A);
        C -> link_left_child(B);
        C -> link_right_child(D);
        E -> link_left_child(C);
        E -> link_right_child(F);
        P -> link_left_child(E);
        E -> rebalance_left<true>();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, nullptr);
        EXPECT_EQ(B -> height, 2);
        EXPECT_EQ(C -> left_child, B);
        EXPECT_EQ(C -> right_child, E);
        EXPECT_EQ(C -> height, 3);

        EXPECT_TRUE(D -> is_leaf());
        EXPECT_EQ(D -> height, 1);
        EXPECT_EQ(E -> left_child, D);
        EXPECT_EQ(E -> right_child, F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);
        EXPECT_EQ(P -> left_child, C);
        EXPECT_EQ(P -> height, 5);
    }

    TEST_F(avl_node_test, rebalance_left_right_basic_test) {
        /*
         *          P
         *         /           P
         *        C           /
         *       /    ->     B
         *      A           / \
         *       \         A   C
         *        B
         */

        node_type* A = make_node(2);
        node_type* B = node_type::construct(allocator);
        node_type* C = make_node(3);
        tree_ptr_type P(make_node(4));

        A -> link_right_child(B);
        C -> link_left_child(A);
        P -> link_left_child(C);
        C -> rebalance_left<true>();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> left_child, B);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rebalance_left_right_complex_test) {
        /*
         *             P   
         *            /              P
         *           E              /
         *          / \            D
         *         B   F    ->    / \
         *        / \            B   E
         *       A   D          / \   \
         *          /          A   C   F
         *         C    
         */

        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(3);
        node_type* C = node_type::construct(allocator);
        node_type* D = make_node(2);
        node_type* E = make_node(4);
        node_type* F = node_type::construct(allocator);
        tree_ptr_type P(make_node(5));

        B -> link_left_child(A);
        B -> link_right_child(D);
        D -> link_left_child(C);
        E -> link_left_child(B);
        E -> link_right_child(F);
        P -> link_left_child(E);
        E -> rebalance_left<true>();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);

        EXPECT_EQ(D -> left_child, B);
        EXPECT_EQ(D -> right_child, E);
        EXPECT_EQ(D -> height, 3);
        EXPECT_EQ(E -> left_child, nullptr);
        EXPECT_EQ(E -> right_child, F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);
        EXPECT_EQ(P -> left_child, D);
        EXPECT_EQ(P -> height, 5);
    }

    TEST_F(avl_node_test, rebalance_right_right_basic_test) {
        /*
         *  P
         *   \           P
         *    A           \
         *     \    ->     B
         *      B         / \
         *       \       A   C
         *        C
         */


        node_type* A = make_node(3);
        node_type* B = make_node(2);
        node_type* C = node_type::construct(allocator);
        tree_ptr_type P(make_node(4));

        A -> link_right_child(B);    
        B -> link_right_child(C);
        P -> link_right_child(A);
        A -> rebalance_right<true>();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> right_child, B);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rebalance_right_right_complex_test) {
        /*
         *   P
         *    \                P
         *     B                \
         *    / \                D
         *   A   D        ->    / \
         *      / \            B   E
         *     C   E          / \   \ 
         *          \        A   C   F
         *           F
         */

        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(4);
        node_type* C = node_type::construct(allocator);
        node_type* D = make_node(3);
        node_type* E = make_node(2);
        node_type* F = node_type::construct(allocator);
        tree_ptr_type P(make_node(5));

        B -> link_left_child(A);
        B -> link_right_child(D);
        D -> link_left_child(C);
        D -> link_right_child(E);
        E -> link_right_child(F);
        P -> link_right_child(B);
        B -> rebalance_right<true>();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);

        EXPECT_EQ(D -> left_child, B);
        EXPECT_EQ(D -> right_child, E);
        EXPECT_EQ(D -> height, 3);
        EXPECT_EQ(E -> right_child, F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);

        EXPECT_EQ(P -> right_child, D);
        EXPECT_EQ(P -> height, 5);
    }

    TEST_F(avl_node_test, rebalance_right_left_basic_test) {
        /*
         *      P
         *       \            P
         *        A            \
         *         \     ->     B
         *          C          / \
         *         /          A   C
         *        B
         */

        node_type* C = make_node(2);
        node_type* B = node_type::construct(allocator);
        node_type* A = make_node(3);
        tree_ptr_type P(make_node(4));

        A -> link_right_child(C);
        C -> link_left_child(B);
        P -> link_right_child(A);
        A -> rebalance_right<true>();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> right_child, C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> right_child, B);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rebalance_right_left_complex_test) {
        /*
         *   P                 P
         *    \                 \
         *     B                 C
         *    / \               / \
         *   A   E     ->      B   E
         *      / \           /   / \
         *     C   F         A   D   F
         *      \
         *       D
         */

        node_type* A = node_type::construct(allocator);
        node_type* B = make_node(4);
        node_type* C = make_node(2);
        node_type* D = node_type::construct(allocator);
        node_type* E = make_node(3);
        node_type* F = node_type::construct(allocator);
        tree_ptr_type P(make_node(5));

        B -> link_left_child(A);
        B -> link_right_child(E);
        C -> link_right_child(D);
        E -> link_left_child(C);
        E -> link_right_child(F);
        P -> link_right_child(B);
        B -> rebalance_right<true>();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child, A);
        EXPECT_EQ(B -> height, 2);
        EXPECT_EQ(C -> left_child, B);
        EXPECT_EQ(C -> right_child, E);
        EXPECT_EQ(C -> height, 3);

        EXPECT_TRUE(D -> is_leaf());
        EXPECT_EQ(D -> height, 1);
        EXPECT_EQ(E -> left_child, D);
        EXPECT_EQ(E -> right_child, F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);
        EXPECT_EQ(P -> right_child, C);
        EXPECT_EQ(P -> height, 5);
    }
}
