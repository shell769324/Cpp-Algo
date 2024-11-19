#include <memory>
#include "tst/tree/tree_test_util.h"
#include <iostream>
#include "tst/utility/tracking_allocator.h"

namespace algo {
    void is_parent_left_child_test(const binary_tree_node<constructor_stub>* node, const binary_tree_node<constructor_stub>* left_child) {
        EXPECT_EQ(node -> left_child.get(), left_child);
        EXPECT_EQ(left_child -> parent, node);
    }

    void is_parent_right_child_test(const binary_tree_node<constructor_stub>* node, const binary_tree_node<constructor_stub>* right_child) {
        EXPECT_EQ(node -> right_child.get(), right_child);
        EXPECT_EQ(right_child -> parent, node);
    }

    binary_tree_node<constructor_stub>::unique_ptr_type create_perfectly_balance_tree(int size, int offset) {
        static tracking_allocator<binary_tree_node<constructor_stub>> allocator;
        if (size == 0) {
            return nullptr;
        }
        int root_val = size / 2;
        auto root = binary_tree_node<constructor_stub>::unique_ptr_type(
            binary_tree_node<constructor_stub>::construct(allocator, constructor_stub{root_val + offset}));
        auto left_child = create_perfectly_balance_tree(root_val, offset);
        if (left_child) {
            root -> link_left_child(std::move(left_child));
        }
        auto right_child = create_perfectly_balance_tree(root_val, offset + root_val + 1);
        if (right_child) {
            root -> link_right_child(std::move(right_child));
        }
        return root;
    }
}
