#include "binary_tree_node_util.h"

namespace algo {
    void singleton_cleaner::operator()(node_type* node) {
        node -> destroy(allocator);
    }

    void tree_cleaner::operator()(node_type* node) {
        node -> deep_destroy(allocator);
    }
}