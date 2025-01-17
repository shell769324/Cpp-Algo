#pragma once
#include "src/tree/binary_tree_node.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/tracking_allocator.h"

namespace algo {
    using node_type = binary_tree_node<constructor_stub>;

    struct singleton_cleaner {
        inline static tracking_allocator<node_type> allocator;
        void operator()(node_type* node);
    };

    struct tree_cleaner {
        inline static tracking_allocator<node_type> allocator;
        void operator()(node_type* node);
    };

    using singleton_ptr_type = std::unique_ptr<node_type, singleton_cleaner>;
    using tree_ptr_type = std::unique_ptr<node_type, tree_cleaner>;

    template <typename... Args>
    singleton_ptr_type make_singleton(Args&&... args) {
        static tracking_allocator<node_type> allocator;
        node_type* node = node_type::construct(allocator, std::forward<Args>(args)...);
        return singleton_ptr_type(node);
    }

    template <typename... Args>
    tree_ptr_type make_tree(Args&&... args) {
        static tracking_allocator<node_type> allocator;
        node_type* node = node_type::construct(allocator, std::forward<Args>(args)...);
        return tree_ptr_type(node);
    }
}