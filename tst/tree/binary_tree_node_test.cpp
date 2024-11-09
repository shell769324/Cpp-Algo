#include "gtest/gtest.h"
#include "tree/binary_tree_node.h"
#include "tst/utility/constructor_stub.h"
#include "tst/tree/tree_test_util.h"


namespace {
    using namespace algo;
    class binary_tree_node_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int BIG_LIMIT = 1 << 8;
    static const int SMALL_LIMIT = 1 << 4;

    using node_type = binary_tree_node<constructor_stub>;

    bool is_equal_nodes(const node_type* node1, const node_type* node2) {
        if (node1 == nullptr || node2 == nullptr) {
            return node1 == nullptr && node2 == nullptr;
        }
        return node1 -> value.id == node2 -> value.id &&
               is_equal_nodes(node1 -> left_child.get(), node2 -> left_child.get()) &&
               is_equal_nodes(node1 -> right_child.get(), node2 -> right_child.get());
    }

    TEST_F(binary_tree_node_test, default_constructor_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        is_missing_parent_children(node.get());
    }

    TEST_F(binary_tree_node_test, perfect_forward_constructor_args_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        is_missing_parent_children(node.get());
        EXPECT_EQ(node -> value.id, SPECIAL_VALUE);
    }

    TEST_F(binary_tree_node_test, perfect_forward_constructor_lvalue_test) {
        constructor_stub stub(SPECIAL_VALUE);
        std::unique_ptr<node_type> node = std::make_unique<node_type>(stub);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        is_missing_parent_children(node.get());
        EXPECT_EQ(node -> value, stub);
    }

    TEST_F(binary_tree_node_test, perfect_forward_constructor_rvalue_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>(constructor_stub{SPECIAL_VALUE});
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        is_missing_parent_children(node.get());
        EXPECT_EQ(node -> value.id, SPECIAL_VALUE);
    }

    TEST_F(binary_tree_node_test, is_root_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        EXPECT_TRUE(node -> is_root());
    }

    TEST_F(binary_tree_node_test, is_leaf_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        EXPECT_TRUE(node -> is_leaf());
    }

    TEST_F(binary_tree_node_test, is_left_child_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> left_child = std::make_unique<node_type>();
        node -> right_child = std::make_unique<node_type>();
        node -> left_child -> parent = node.get();
        node -> right_child -> parent = node.get();
        EXPECT_TRUE(node -> left_child -> is_left_child());
        EXPECT_FALSE(node -> right_child -> is_left_child());
    }

    TEST_F(binary_tree_node_test, is_leaf_complex_test) {
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        EXPECT_TRUE(node -> is_leaf());
        node_type* child = new node_type();
        node -> link_left_child(child);
        EXPECT_FALSE(node -> is_leaf());
        child -> orphan_self();
        node -> link_right_child(child);
        EXPECT_FALSE(node -> is_leaf());
        node_type* child2 = new node_type();
        node -> link_left_child(child2);
        EXPECT_FALSE(node -> is_leaf());
    }

    TEST_F(binary_tree_node_test, link_left_child_raw_ptr_test) {
        node_type* left_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> link_left_child(left_child);
        is_parent_left_child_test(node.get(), left_child);
        EXPECT_EQ(prev_child, nullptr);
    }

    TEST_F(binary_tree_node_test, safe_link_left_child_raw_ptr_test) {
        node_type* left_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> safe_link_left_child(left_child);
        is_parent_left_child_test(node.get(), left_child);
        EXPECT_EQ(prev_child, nullptr);
        prev_child = node -> safe_link_left_child(nullptr);
        EXPECT_EQ(node -> left_child.get(), nullptr);
        EXPECT_EQ(left_child -> parent, nullptr);
        EXPECT_EQ(prev_child, left_child);
        delete prev_child;
    }

    TEST_F(binary_tree_node_test, link_left_child_raw_ptr_replacing_test) {
        node_type* left_child1 = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_left_child(left_child1);
        node_type* left_child2 = new node_type();
        node_type* prev_child = node -> link_left_child(left_child2);
        is_parent_left_child_test(node.get(), left_child2);
        EXPECT_EQ(prev_child, left_child1);
        EXPECT_EQ(left_child1 -> parent, nullptr);
        delete left_child1;
    }

    TEST_F(binary_tree_node_test, link_left_child_unique_ptr_test) {
        node_type* left_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> link_left_child(std::unique_ptr<node_type>(left_child));
        is_parent_left_child_test(node.get(), left_child);
        EXPECT_EQ(prev_child, nullptr);
    }

    TEST_F(binary_tree_node_test, safe_link_left_child_unique_ptr_test) {
        node_type* left_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> safe_link_left_child(std::unique_ptr<node_type>(left_child));
        is_parent_left_child_test(node.get(), left_child);
        EXPECT_EQ(prev_child, nullptr);
        prev_child = node -> safe_link_left_child(std::unique_ptr<node_type>(nullptr));
        EXPECT_EQ(node -> left_child.get(), nullptr);
        EXPECT_EQ(left_child -> parent, nullptr);
        EXPECT_EQ(prev_child, left_child);
        delete left_child;
    }

    TEST_F(binary_tree_node_test, link_left_child_unique_ptr_replacing_test) {
        node_type* left_child1 = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_left_child(std::unique_ptr<node_type>(left_child1));
        node_type* left_child2 = new node_type();
        node_type* prev_child = node -> link_left_child(std::unique_ptr<node_type>(left_child2));
        is_parent_left_child_test(node.get(), left_child2);
        EXPECT_EQ(prev_child, left_child1);
        EXPECT_EQ(left_child1 -> parent, nullptr);
        delete left_child1;
    }

    TEST_F(binary_tree_node_test, link_right_child_raw_ptr_test) {
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> link_right_child(right_child);
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_child, nullptr);
    }

    TEST_F(binary_tree_node_test, safe_link_right_child_raw_ptr_test) {
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> safe_link_right_child(right_child);
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_child, nullptr);
        prev_child = node -> safe_link_right_child(nullptr);
        EXPECT_EQ(node -> right_child.get(), nullptr);
        EXPECT_EQ(right_child -> parent, nullptr);
        EXPECT_EQ(prev_child, right_child);
        delete prev_child;
    }

    TEST_F(binary_tree_node_test, link_right_child_raw_ptr_replacing_test) {
        node_type* right_child1 = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_right_child(right_child1);
        node_type* right_child2 = new node_type();
        node_type* prev_child = node -> link_right_child(right_child2);
        is_parent_right_child_test(node.get(), right_child2);
        EXPECT_EQ(prev_child, right_child1);
        EXPECT_EQ(right_child1 -> parent, nullptr);
        delete right_child1;
    }

    TEST_F(binary_tree_node_test, link_right_child_unique_ptr_test) {
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> link_right_child(std::unique_ptr<node_type>(right_child));
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_child, nullptr);
    }

    TEST_F(binary_tree_node_test, safe_link_right_child_unique_ptr_test) {
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_child = node -> safe_link_right_child(std::unique_ptr<node_type>(right_child));
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_child, nullptr);
        prev_child = node -> safe_link_right_child(std::unique_ptr<node_type>(nullptr));
        EXPECT_EQ(node -> right_child.get(), nullptr);
        EXPECT_EQ(right_child -> parent, nullptr);
        EXPECT_EQ(prev_child, right_child);
        delete right_child;
    }

    TEST_F(binary_tree_node_test, link_right_child_unique_ptr_replacing_test) {
        node_type* right_child1 = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_right_child(std::unique_ptr<node_type>(right_child1));
        node_type* right_child2 = new node_type();
        node_type* prev_child = node -> link_right_child(std::unique_ptr<node_type>(right_child2));
        is_parent_right_child_test(node.get(), right_child2);
        EXPECT_EQ(prev_child, right_child1);
        EXPECT_EQ(right_child1 -> parent, nullptr);
        delete right_child1;
    }

    TEST_F(binary_tree_node_test, link_child_raw_ptr_test) {
        node_type* left_child = new node_type();
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_left_child = node -> link_child(left_child, true);
        node_type* prev_right_child = node -> link_child(right_child, false);
        is_parent_left_child_test(node.get(), left_child);
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_left_child, nullptr);
        EXPECT_EQ(prev_right_child, nullptr);
    }

    TEST_F(binary_tree_node_test, safe_link_child_raw_ptr_test) {
        node_type* left_child = new node_type();
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_left_child = node -> safe_link_child(left_child, true);
        node_type* prev_right_child = node -> safe_link_child(right_child, false);
        is_parent_left_child_test(node.get(), left_child);
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_left_child, nullptr);
        EXPECT_EQ(prev_right_child, nullptr);
        prev_left_child = node -> safe_link_child(nullptr, true);
        prev_right_child = node -> safe_link_child(nullptr, false);
        
        EXPECT_EQ(node -> left_child.get(), nullptr);
        EXPECT_EQ(left_child -> parent, nullptr);
        EXPECT_EQ(prev_left_child, left_child);

        EXPECT_EQ(node -> right_child.get(), nullptr);
        EXPECT_EQ(right_child -> parent, nullptr);
        EXPECT_EQ(prev_right_child, right_child);   
        delete prev_left_child;
        delete prev_right_child;
    }

    TEST_F(binary_tree_node_test, link_child_raw_ptr_replacing_test) {
        node_type* left_child1 = new node_type();
        node_type* right_child1 = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_child(left_child1, true);
        node -> link_child(right_child1, false);
        node_type* left_child2 = new node_type();
        node_type* right_child2 = new node_type();
        node_type* prev_left_child = node -> link_child(left_child2, true);
        node_type* prev_right_child = node -> link_child(right_child2, false);
        is_parent_left_child_test(node.get(), left_child2);
        is_parent_right_child_test(node.get(), right_child2);
        EXPECT_EQ(prev_left_child, left_child1);
        EXPECT_EQ(left_child1 -> parent, nullptr);
        EXPECT_EQ(prev_right_child, right_child1);
        EXPECT_EQ(right_child1 -> parent, nullptr);
        delete left_child1;
        delete right_child1;
    }

    TEST_F(binary_tree_node_test, link_child_unique_ptr_test) {
        node_type* left_child = new node_type();
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_left_child = node -> link_child(std::unique_ptr<node_type>(left_child), true);
        node_type* prev_right_child = node -> link_child(std::unique_ptr<node_type>(right_child), false);
        is_parent_left_child_test(node.get(), left_child);
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_left_child, nullptr);
        EXPECT_EQ(prev_right_child, nullptr);
    }

    TEST_F(binary_tree_node_test, safe_link_child_unique_ptr_test) {
        node_type* left_child = new node_type();
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node_type* prev_left_child = node -> safe_link_child(std::unique_ptr<node_type>(left_child), true);
        node_type* prev_right_child = node -> safe_link_child(std::unique_ptr<node_type>(right_child), false);
        is_parent_left_child_test(node.get(), left_child);
        is_parent_right_child_test(node.get(), right_child);
        EXPECT_EQ(prev_left_child, nullptr);
        EXPECT_EQ(prev_right_child, nullptr);

        prev_left_child = node -> safe_link_child(std::unique_ptr<node_type>(nullptr), true);
        prev_right_child = node -> safe_link_child(std::unique_ptr<node_type>(nullptr), false);
        
        EXPECT_EQ(node -> left_child.get(), nullptr);
        EXPECT_EQ(left_child -> parent, nullptr);
        EXPECT_EQ(prev_left_child, left_child);

        EXPECT_EQ(node -> right_child.get(), nullptr);
        EXPECT_EQ(right_child -> parent, nullptr);
        EXPECT_EQ(prev_right_child, right_child);   
        delete prev_left_child;
        delete prev_right_child;
    }

    TEST_F(binary_tree_node_test, link_child_unique_ptr_replacing_test) {
        node_type* left_child1 = new node_type();
        node_type* right_child1 = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_child(std::unique_ptr<node_type>(left_child1), true);
        node -> link_child(std::unique_ptr<node_type>(right_child1), false);
        node_type* left_child2 = new node_type();
        node_type* right_child2 = new node_type();
        node_type* prev_left_child = node -> link_child(std::unique_ptr<node_type>(left_child2), true);
        node_type* prev_right_child = node -> link_child(std::unique_ptr<node_type>(right_child2), false);
        is_parent_left_child_test(node.get(), left_child2);
        is_parent_right_child_test(node.get(), right_child2);
        EXPECT_EQ(prev_left_child, left_child1);
        EXPECT_EQ(left_child1 -> parent, nullptr);
        EXPECT_EQ(prev_right_child, right_child1);
        EXPECT_EQ(right_child1 -> parent, nullptr);
        delete left_child1;
        delete right_child1;
    }

    TEST_F(binary_tree_node_test, orphan_self_test) {
        node_type* left_child = new node_type();
        node_type* right_child = new node_type();
        std::unique_ptr<node_type> node = std::make_unique<node_type>();
        node -> link_left_child(left_child);
        node -> link_right_child(right_child);
        left_child -> orphan_self();
        right_child -> orphan_self();
        EXPECT_EQ(node -> left_child.get(), nullptr);
        EXPECT_EQ(left_child -> parent, nullptr);
        EXPECT_EQ(node -> right_child.get(), nullptr);
        EXPECT_EQ(right_child -> parent, nullptr);
        delete left_child;
        delete right_child;
    }

    TEST_F(binary_tree_node_test, get_leftmost_descendant_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        EXPECT_EQ(root -> get_leftmost_descendant() -> value.id, 0);
    }

    TEST_F(binary_tree_node_test, get_rightmost_descendant_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        EXPECT_EQ(root -> get_rightmost_descendant() -> value.id, SMALL_LIMIT - 2);
    }

    TEST_F(binary_tree_node_test, next_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        iterable_node<constructor_stub>* curr = root -> get_leftmost_descendant();
        for (int i = 0; i < SMALL_LIMIT - 1; ++i) {
            EXPECT_EQ(curr -> value.id, i);
            curr = curr -> next();
        }
        EXPECT_EQ(curr, nullptr);
    }

    TEST_F(binary_tree_node_test, prev_test) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(SMALL_LIMIT - 1, 0);
        iterable_node<constructor_stub>* curr = root -> get_rightmost_descendant();
        for (int i = SMALL_LIMIT - 2; i >= 0; --i) {
            EXPECT_EQ(curr -> value.id, i);
            curr = curr -> prev();
        }
        EXPECT_EQ(curr, nullptr);
    }

    void deep_clone_test(int size) {
        std::unique_ptr<node_type> root = create_perfectly_balance_tree(size, 0);
        std::unique_ptr<node_type> root_copy(root -> deep_clone());
        EXPECT_TRUE(is_equal_nodes(root.get(), root_copy.get()));
    }

    TEST_F(binary_tree_node_test, deep_clone_basic_test) {
        deep_clone_test(1);
    }

    TEST_F(binary_tree_node_test, deep_clone_stress_test) {
        deep_clone_test(BIG_LIMIT - 1);
    }
}
