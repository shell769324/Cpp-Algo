#pragma once
#include "binary_tree_node.h"
#include "binary_tree_common.h"
#include "binary_tree_iterator.h"
#include "src/thread_pool_executor/thread_pool_executor.h"
#include <functional>

namespace algo {

/**
 * @brief Base class for all binary tree classes
 * 
 * @tparam K the type of the key
 * @tparam V the type of the value
 * @tparam KeyOf the key extractor function or functor
 * @tparam Comparator the key comparator function of functor
 */
template <typename K, typename V, typename KeyOf, typename NodeType, typename TreeType, typename Comparator = std::less<K> >
    requires binary_tree_definable<K, V, KeyOf, Comparator>
class binary_tree_base {

protected:
    KeyOf key_of;
    Comparator comp;

    using key_type = K;
    using value_type = V;
    using reference = V&;
    using const_reference = const V&;
    using iterator = binary_tree_iterator<V>;
    using const_iterator = binary_tree_iterator<const V>;
    using reverse_iterator = binary_tree_iterator<V, true>;
    using const_reverse_iterator = const binary_tree_iterator<const V, true>;
    using node_type = NodeType;
    using smart_ptr_type = std::unique_ptr<NodeType>;


    constexpr static const char EXISTS = 0x2;
    constexpr static const char IS_LEFT_CHILD = 0x1;

    /**
     * @brief Compare two keys
     * @return -1 if k1 < k2,
     *         0 if k1 == k2,
     *         1 if k1 > k2
     */
    int key_comp(const K& k1, const K& k2) const noexcept {
        if (comp(k1, k2)) {
            return -1;
        }
        return comp(k2, k1) ? 1 : 0;
    }

public:
    binary_tree_base() = default;

    binary_tree_base(const Comparator& comp) : comp(comp) { }

    binary_tree_base(Comparator&& comp) : comp(std::move(comp)) { }

    /**
     * @brief Get a pointer to the underlying node type
     */
    TreeType* underlying_ptr() noexcept {
        return static_cast<TreeType*>(this);
    }

    NodeType* find(NodeType* root, const K& key) const {
        NodeType* curr = root;
        while (curr != nullptr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return curr;
            }
            if (result < 0) {
                curr = curr -> left_child.get();
            } else {
                curr = curr -> right_child.get();
            }
        }
        return nullptr;
    }

    NodeType* max_leq(NodeType* root, const K& key) const {
        NodeType* curr = root;
        NodeType* res = nullptr;
        while (curr != nullptr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return curr;
            }
            if (result < 0) {
                curr = curr -> left_child.get();
            } else {
                res = curr;
                curr = curr -> right_child.get();
            }
        }
        return res;
    }

    NodeType* min_geq(NodeType* root, const K& key) const {
        NodeType* curr = root;
        NodeType* res = nullptr;
        while (curr != nullptr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return curr;
            }
            if (result < 0) {
                res = curr;
                curr = curr -> left_child.get();
            } else {
                curr = curr -> right_child.get();
            }
        }
        return res;
    }

    /**
     * @brief Get the node that would be the parent of the new node when the
     *        given key is inserted
     * 
     * No insertion is performed
     * 
     * @param root the root of the tree
     * @param key the key to insert
     * @return std::pair<NodeType*, char> a pair of the node and a char flag
     * 
     * The flag has two fields, the second to the least significant bit is true if
     * the key already exists. In this case, the least significant bit will be false
     * The node will be the node that has the equal key
     * 
     * Otherwise, the least significant bit is true if the new node would be the left
     * child of the parent node. It is false otherwise. The node will be the would-be parent
     */
    std::pair<NodeType*, char> get_insertion_parent(NodeType* root, const K& key) const {
        if (!root) {
            throw std::invalid_argument("root must not be empty");
        }
        NodeType* curr = root;
        while (curr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return std::make_pair(curr, EXISTS);
            }
            NodeType* next_node;
            if (result < 0) {
                next_node = curr -> left_child.get();
                if (!next_node) {
                    return std::make_pair(curr, IS_LEFT_CHILD);
                }
            } else {
                next_node = curr -> right_child.get();
                if (!next_node) {
                    return std::make_pair(curr, 0);
                }
            }
            curr = next_node;
        }
        throw std::range_error("Impossible state");
    }

private:
    /**
     * @brief An higher order function that combines two rooted trees according to a combinator
     * 
     * @tparam Resolver the type of the function that resolves conflicts when the same key
     *         is found in both trees
     * @tparam Combinator the type of the function that determines how trees are combined
     * @param root1 the root of the first tree
     * @param root2 the root of the second tree
     * @param resolver the function that resolves conflicts when the same key is found in both trees
     * @param combinator the function that determines how trees are combined
     * @return smart_ptr_type the tree after combining the two trees
     */
    template<typename Resolver, typename Combinator>
    smart_ptr_type set_operation(smart_ptr_type root1, smart_ptr_type root2, Resolver& resolver,
        Combinator& combinator) {
        if (!root1 || !root2) {
            if (combinator(root1 != nullptr, root2 != nullptr)) {
                return root1 ? std::move(root1) : std::move(root2);
            }
            return nullptr;
        }
        node_type* left1 = root1 -> orphan_left_child();
        node_type* right1 = root1 -> orphan_right_child();
        std::pair<smart_ptr_type, bool> split_result(underlying_ptr() -> split(std::move(root2), std::move(root1), resolver));
        smart_ptr_type split_root = std::move(split_result.first);

        smart_ptr_type result_left(set_operation(smart_ptr_type(left1), std::move(split_root -> left_child), resolver, combinator));
        smart_ptr_type result_right(set_operation(smart_ptr_type(right1), std::move(split_root -> right_child), resolver, combinator));

        if (combinator(true, split_result.second)) {
            return TreeType::join(std::move(result_left), std::move(split_root), std::move(result_right));
        }
        return TreeType::join(std::move(result_left), std::move(result_right));
    }

    template<typename Resolver, typename Combinator>
    smart_ptr_type set_operation(smart_ptr_type root1, smart_ptr_type root2, thread_pool_executor& executor,
        Resolver& resolver, Combinator& combinator) {
        if (!root1 || !root2) {
            if (combinator(root1 != nullptr, root2 != nullptr)) {
                return root1 ? std::move(root1) : std::move(root2);
            }
            return nullptr;
        }
        node_type* left1 = root1 -> orphan_left_child();
        node_type* right1 = root1 -> orphan_right_child();
        std::pair<smart_ptr_type, bool> split_result(underlying_ptr() -> split(std::move(root2), std::move(root1), resolver));
        smart_ptr_type split_root = std::move(split_result.first);

        std::future<smart_ptr_type> future;
        bool is_parallel = should_parallelize(left1, split_root -> left_child.get());
        
        smart_ptr_type result_left;
        smart_ptr_type result_right;
        if (is_parallel) {
            auto lambda = [this](smart_ptr_type root1, smart_ptr_type root2,
                thread_pool_executor& executor, Resolver& resolver, Combinator& combinator) {
                return set_operation(std::move(root1), std::move(root2), executor, resolver, combinator);
            };
            task<smart_ptr_type> task(
                lambda, smart_ptr_type(left1), std::move(split_root -> left_child), executor, resolver, combinator);
            future = task.get_future();
            executor.attempt_parallel(std::move(task));
        } else {
            result_left = set_operation(smart_ptr_type(left1), std::move(split_root -> left_child), resolver, combinator);
            result_right = set_operation(smart_ptr_type(right1), std::move(split_root -> right_child), resolver, combinator);
        }

        if (is_parallel) {
            result_right = set_operation(smart_ptr_type(right1), std::move(split_root -> right_child), executor, resolver, combinator);
            result_left = future.get();
        }
        if (combinator(true, split_result.second)) {
            return TreeType::join(std::move(result_left), std::move(split_root), std::move(result_right));
        }
        return TreeType::join(std::move(result_left), std::move(result_right));
    }

public:

    /**
     * @brief A helper function for computing union of two trees
     */
    template<typename Resolver>
    smart_ptr_type union_of(smart_ptr_type root1, smart_ptr_type root2, Resolver& resolver) {
        constexpr auto combinator = std::logical_or();
        return set_operation(std::move(root1), std::move(root2), resolver, combinator);
    }

    template<typename Resolver>
    smart_ptr_type union_of(smart_ptr_type root1, smart_ptr_type root2, thread_pool_executor& executor,
        Resolver& resolver) {
        constexpr auto combinator = std::logical_or();
        return set_operation(std::move(root1), std::move(root2), executor, resolver, combinator);
    }

    /**
     * @brief A helper function for computing intersection of two trees
     */
    template<typename Resolver>
    smart_ptr_type intersection_of(smart_ptr_type root1, smart_ptr_type root2, Resolver& resolver) {
        constexpr auto combinator = std::logical_and();
        return set_operation(std::move(root1), std::move(root2), resolver, combinator);
    }

    template<typename Resolver>
    smart_ptr_type intersection_of(smart_ptr_type root1, smart_ptr_type root2, thread_pool_executor& executor,
    Resolver& resolver) {
        constexpr auto combinator = std::logical_and();

        return set_operation(std::move(root1), std::move(root2), executor, resolver, combinator);
    }
    
    /**
     * @brief Helper method for computing difference of two trees
     */
    smart_ptr_type difference_of(smart_ptr_type root1, smart_ptr_type root2) {
        constexpr auto combinator = [](bool contains1, bool contains2) {
            return contains1 && !contains2;
        };
        chooser<value_type> dummyResolver;
        return set_operation(std::move(root1), std::move(root2), dummyResolver, combinator);
    }

    smart_ptr_type difference_of(smart_ptr_type root1, smart_ptr_type root2, thread_pool_executor& executor) {
        constexpr auto combinator = [](bool contains1, bool contains2) {
            return contains1 && !contains2;
        };
        chooser<value_type> dummyResolver;
        return set_operation(std::move(root1), std::move(root2), executor, dummyResolver, combinator);
    }

    template<std::forward_iterator ForwardIt>
    bool is_inorder(ForwardIt begin, ForwardIt end) const noexcept {
        if (begin == end) {
            return true;
        }
        for (ForwardIt it = begin; std::next(it) != end; ++it) {
            ForwardIt successor = std::next(it);
            if (this -> key_comp(this -> key_of(*it), this -> key_of(*successor)) > 0) {
                std::cout << "is_inorder failure" << std::endl;
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Compute the number of nodes in a tree
     * 
     * @param node the root of the tree
     * @return unsigned the number of nodes in the tree
     */
    unsigned compute_size(NodeType* node) const noexcept {
        if (!node) {
            return 0;
        }
        return 1 + compute_size(node -> left_child.get()) + compute_size(node -> right_child.get());
    }

    /**
     * @brief Check if the size of a tree matches a number
     * 
     * @param node the root of the tree
     * @param expected_size the expected size of the tree
     * @return true iff the tree size matches the expected size
     */
    bool is_size_correct(NodeType* node, unsigned expected_size) const noexcept {
        unsigned actual_size = compute_size(node);
        if (actual_size != expected_size) {
            std::cout << "is_size_correct failure" << std::endl;
            std::cout << "  Actual: " << actual_size << std::endl;
            std::cout << "Expected: " << expected_size << std::endl;
            return false;
        }
        return true;
    }

    /**
     * @brief Check if all nodes of a tree have their parent/children pointers set correctly
     * 
     * @param node the root of the tree
     * @return true iff all
     */
    static bool is_parent_child_link_mutual(const NodeType* node) noexcept {
        if (!node) {
            return true;
        }
        if (node -> left_child) {
            if (node -> left_child -> parent != node) {
                std::cout << "is_parent_child_link_mutual failure" << std::endl;
                std::cout << "left child doesn't point to its parent" << std::endl;
                return false;
            }
        }
        if (node -> right_child) {
            if (node -> right_child -> parent != node) {
                std::cout << "is_parent_child_link_mutual failure" << std::endl;
                std::cout << "right child doesn't point to its parent" << std::endl;
                return false;
            }
        }
        return true;
    }
};

}
