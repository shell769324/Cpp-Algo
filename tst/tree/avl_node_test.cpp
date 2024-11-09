#include "gtest/gtest.h"
#include "tree/avl_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/tree/tree_test_util.h"


namespace {
    using namespace algo;
    class avl_node_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    static const unsigned char SPECIAL_VALUE = 0x3e;
    static const unsigned char SPECIAL_VALUE2 = 0x2a;
    static const int SMALL_LIMIT = 1 << 4;

    using node_type = avl_node<constructor_stub>;

    node_type* make_node(signed char height) {
        node_type* node = new node_type();
        node -> height = height;
        return node;
    }

    
    TEST_F(avl_node_test, default_constructor_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        EXPECT_EQ(node -> height, 1);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(avl_node_test, perfect_forwarding_constructor_args_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(SPECIAL_VALUE);
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(node -> value.id, SPECIAL_VALUE);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(avl_node_test, perfect_forwarding_constructor_lvalue_test) {
        constructor_stub stub(SPECIAL_VALUE);
        std::unique_ptr<node_type> node = std::make_unique<node_type>(stub);
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(node -> value, stub);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(avl_node_test, perfect_forwarding_constructor_rvalue_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(node -> height, 1);
        EXPECT_EQ(node -> value.id, SPECIAL_VALUE);
        is_missing_parent_children(node.get());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
    }

    TEST_F(avl_node_test, clone_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(constructor_stub(SPECIAL_VALUE));
        node -> height = SPECIAL_VALUE;
        std::unique_ptr<node_type> node_copy(node -> clone());
        EXPECT_EQ(node -> height, node_copy -> height);
        EXPECT_EQ(node -> value, node_copy -> value);
        is_missing_parent_children(node_copy.get());
    }

    TEST_F(avl_node_test, compute_balance_factor_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(constructor_stub(SPECIAL_VALUE));
        node -> link_left_child(make_node(SPECIAL_VALUE));
        node -> link_right_child(make_node(SPECIAL_VALUE2));
        EXPECT_EQ((int) node -> compute_balance_factor(), (int) SPECIAL_VALUE - SPECIAL_VALUE2);
    }

    TEST_F(avl_node_test, update_height_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(constructor_stub(SPECIAL_VALUE));
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
        node_type* A = new node_type();
        node_type* B = make_node(3);
        node_type* C;
        node_type* D = make_node(2);
        node_type* E = new node_type();
        std::unique_ptr<node_type> P(make_node(4));

        B -> link_left_child(A);
        B -> link_right_child(D);
        if (has_C) {
            C = new node_type();
            D -> link_left_child(C);
        }
        D -> link_right_child(E);
        P -> link_left_child(B);
        int old_constructor_invocation_count = constructor_stub::constructor_invocation_count;
        B -> rotate_left();
        EXPECT_EQ(old_constructor_invocation_count, constructor_stub::constructor_invocation_count);
        EXPECT_TRUE(A -> is_leaf());
        if (has_C) {
            EXPECT_TRUE(C -> is_leaf());
            EXPECT_EQ(B -> right_child.get(), C);
        } else {
            EXPECT_EQ(B -> right_child.get(), nullptr);
        }
        EXPECT_TRUE(E -> is_leaf());
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> height, 2);
        EXPECT_EQ(D -> left_child.get(), B);
        EXPECT_EQ(D -> height, 3);
        EXPECT_EQ(D -> right_child.get(), E);
        EXPECT_EQ(P -> left_child.get(), D);
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
        node_type* A = new node_type();
        node_type* B = make_node(2);
        node_type* C;
        node_type* D = make_node(3);
        node_type* E = new node_type();
        std::unique_ptr<node_type> P(make_node(4));

        B -> link_left_child(A);
        if (has_C) {
            C = new node_type();
            B -> link_right_child(C);
        }
        D -> link_left_child(B);
        D -> link_right_child(E);
        P -> link_right_child(D);
        int old_constructor_invocation_count = constructor_stub::constructor_invocation_count;
        D -> rotate_right();
        EXPECT_EQ(old_constructor_invocation_count, constructor_stub::constructor_invocation_count);
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_TRUE(E -> is_leaf());
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> height, 3);
        if (has_C) {
            EXPECT_TRUE(C -> is_leaf());
            EXPECT_EQ(D -> left_child.get(), C);
        } else {
            EXPECT_EQ(D -> left_child.get(), nullptr);
        }
        EXPECT_EQ(D -> height, 2);
        EXPECT_EQ(D -> right_child.get(), E);
        EXPECT_EQ(P -> right_child.get(), B);
        EXPECT_EQ(P -> height, 4);
    }

    TEST_F(avl_node_test, rotate_right_full_house_test) {
        rotate_right_test(true);
    }

    TEST_F(avl_node_test, rotate_right_missing_left_right_child_test) {
        rotate_right_test(false);
    }

    void rotate_stress_test(bool has_C) {
        node_type* A = new node_type();
        node_type* B = make_node(2);
        node_type* C;
        node_type* D = make_node(3);
        node_type* E = new node_type();
        std::unique_ptr<node_type> P(make_node(4));

        B -> link_left_child(A);
        if (has_C) {
            C = new node_type();
            B -> link_right_child(C);
        }
        D -> link_left_child(B);
        D -> link_right_child(E);
        P -> link_right_child(D);
        int old_constructor_invocation_count = constructor_stub::constructor_invocation_count;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            D -> rotate_right();
            B -> rotate_left();
        }
        D -> rotate_right();
        EXPECT_EQ(old_constructor_invocation_count, constructor_stub::constructor_invocation_count);
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_TRUE(E -> is_leaf());
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> height, 3);
        if (has_C) {
            EXPECT_TRUE(C -> is_leaf());
            EXPECT_EQ(D -> left_child.get(), C);
        } else {
            EXPECT_EQ(D -> left_child.get(), nullptr);
        }
        EXPECT_EQ(D -> height, 2);
        EXPECT_EQ(D -> right_child.get(), E);
        EXPECT_EQ(P -> right_child.get(), B);
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

        node_type* A = new node_type();
        node_type* B = make_node(2);
        node_type* C = make_node(3);
        std::unique_ptr<node_type> P(make_node(4));

        B -> link_left_child(A);
        C -> link_left_child(B);
        P -> link_left_child(C);
        C -> rebalance_left();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> left_child.get(), B);
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

        node_type* A = new node_type();
        node_type* B = make_node(2);
        node_type* C = make_node(3);
        node_type* D = new node_type();
        node_type* E = make_node(4);
        node_type* F = new node_type();
        std::unique_ptr<node_type> P(make_node(5));

        B -> link_left_child(A);
        C -> link_left_child(B);
        C -> link_right_child(D);
        E -> link_left_child(C);
        E -> link_right_child(F);
        P -> link_left_child(E);
        E -> rebalance_left();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), nullptr);
        EXPECT_EQ(B -> height, 2);
        EXPECT_EQ(C -> left_child.get(), B);
        EXPECT_EQ(C -> right_child.get(), E);
        EXPECT_EQ(C -> height, 3);

        EXPECT_TRUE(D -> is_leaf());
        EXPECT_EQ(D -> height, 1);
        EXPECT_EQ(E -> left_child.get(), D);
        EXPECT_EQ(E -> right_child.get(), F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);
        EXPECT_EQ(P -> left_child.get(), C);
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
        node_type* B = new node_type();
        node_type* C = make_node(3);
        std::unique_ptr<node_type> P(make_node(4));

        A -> link_right_child(B);
        C -> link_left_child(A);
        P -> link_left_child(C);
        C -> rebalance_left();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> left_child.get(), B);
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

        node_type* A = new node_type();
        node_type* B = make_node(3);
        node_type* C = new node_type();
        node_type* D = make_node(2);
        node_type* E = make_node(4);
        node_type* F = new node_type();
        std::unique_ptr<node_type> P(make_node(5));

        B -> link_left_child(A);
        B -> link_right_child(D);
        D -> link_left_child(C);
        E -> link_left_child(B);
        E -> link_right_child(F);
        P -> link_left_child(E);
        E -> rebalance_left();
        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);

        EXPECT_EQ(D -> left_child.get(), B);
        EXPECT_EQ(D -> right_child.get(), E);
        EXPECT_EQ(D -> height, 3);
        EXPECT_EQ(E -> left_child.get(), nullptr);
        EXPECT_EQ(E -> right_child.get(), F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);
        EXPECT_EQ(P -> left_child.get(), D);
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
        node_type* C = new node_type();
        std::unique_ptr<node_type> P(make_node(4));

        A -> link_right_child(B);    
        B -> link_right_child(C);
        P -> link_right_child(A);
        A -> rebalance_right();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> right_child.get(), B);
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

        node_type* A = new node_type();
        node_type* B = make_node(4);
        node_type* C = new node_type();
        node_type* D = make_node(3);
        node_type* E = make_node(2);
        node_type* F = new node_type();
        std::unique_ptr<node_type> P(make_node(5));

        B -> link_left_child(A);
        B -> link_right_child(D);
        D -> link_left_child(C);
        D -> link_right_child(E);
        E -> link_right_child(F);
        P -> link_right_child(B);
        B -> rebalance_right();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);

        EXPECT_EQ(D -> left_child.get(), B);
        EXPECT_EQ(D -> right_child.get(), E);
        EXPECT_EQ(D -> height, 3);
        EXPECT_EQ(E -> right_child.get(), F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);

        EXPECT_EQ(P -> right_child.get(), D);
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
        node_type* B = new node_type();
        node_type* A = make_node(3);
        std::unique_ptr<node_type> P(make_node(4));

        A -> link_right_child(C);
        C -> link_left_child(B);
        P -> link_right_child(A);
        A -> rebalance_right();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> right_child.get(), C);
        EXPECT_EQ(B -> height, 2);
        EXPECT_TRUE(C -> is_leaf());
        EXPECT_EQ(C -> height, 1);
        EXPECT_EQ(P -> right_child.get(), B);
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

        node_type* A = new node_type();
        node_type* B = make_node(4);
        node_type* C = make_node(2);
        node_type* D = new node_type();
        node_type* E = make_node(3);
        node_type* F = new node_type();
        std::unique_ptr<node_type> P(make_node(5));

        B -> link_left_child(A);
        B -> link_right_child(E);
        C -> link_right_child(D);
        E -> link_left_child(C);
        E -> link_right_child(F);
        P -> link_right_child(B);
        B -> rebalance_right();

        EXPECT_TRUE(A -> is_leaf());
        EXPECT_EQ(A -> height, 1);
        EXPECT_EQ(B -> left_child.get(), A);
        EXPECT_EQ(B -> height, 2);
        EXPECT_EQ(C -> left_child.get(), B);
        EXPECT_EQ(C -> right_child.get(), E);
        EXPECT_EQ(C -> height, 3);

        EXPECT_TRUE(D -> is_leaf());
        EXPECT_EQ(D -> height, 1);
        EXPECT_EQ(E -> left_child.get(), D);
        EXPECT_EQ(E -> right_child.get(), F);
        EXPECT_EQ(E -> height, 2);
        EXPECT_TRUE(F -> is_leaf());
        EXPECT_EQ(F -> height, 1);
        EXPECT_EQ(P -> right_child.get(), C);
        EXPECT_EQ(P -> height, 5);
    }
}
