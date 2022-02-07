#pragma once
#include <memory>
#include "tree/binary_tree_node.h"
#include "tst/utility/constructor_stub.h"
#include "gtest/gtest.h"

namespace algo {
    
    template <typename Derived>
    void is_missing_parent_children(const binary_tree_node_base<constructor_stub, Derived>* node) {
        EXPECT_EQ(node -> left_child.get(), nullptr);
        EXPECT_EQ(node -> right_child.get(), nullptr);
        EXPECT_EQ(node -> parent, nullptr);
    }

    void is_parent_left_child_test(const binary_tree_node<constructor_stub>* node, const binary_tree_node<constructor_stub>* left_child);

    void is_parent_right_child_test(const binary_tree_node<constructor_stub>* node, const binary_tree_node<constructor_stub>* right_child);

    template <typename T, typename Derived>
    int compute_height(const binary_tree_node_base<T, Derived>* node) {
        if (node == nullptr) {
            return 0;
        }
        return std::max(compute_height(node -> left_child.get()), compute_height(node -> right_child.get())) + 1;
    }

    std::unique_ptr<binary_tree_node<constructor_stub> > create_perfectly_balance_tree(int size, int offset);
}