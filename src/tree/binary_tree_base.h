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
 * @tparam Compare the key comparator function of functor
 */
template <typename K, typename V, typename KeyOf, typename NodeType, typename TreeType, typename Compare = std::less<K>, typename Allocator = std::allocator<V> >
    requires binary_tree_definable<K, V, KeyOf, Compare, Allocator>
class binary_tree_base {
private:
    using alloc_traits = std::allocator_traits<Allocator>;

protected:
    using key_type = K;
    using value_type = V;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using key_compare = Compare;
    using allocator_type = Allocator;
    using reference = V&;
    using const_reference = const V&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using iterator = binary_tree_iterator<NodeType>;
    using const_iterator = binary_tree_iterator<constify<value_type, NodeType> >;
    using reverse_iterator = binary_tree_iterator<NodeType, true>;
    using const_reverse_iterator = binary_tree_iterator<constify<value_type, NodeType>, true>;

    using node_type = NodeType;
    using node_allocator_type = typename alloc_traits::rebind_alloc<node_type>;
    using node_alloc_traits = typename alloc_traits::rebind_traits<node_type>;
    using unique_ptr_type = std::unique_ptr<node_type, stub_deleter<node_type> >;

    KeyOf key_of;
    Compare comp;
    node_allocator_type node_allocator;


    constexpr static const char EXISTS = 0x2;
    constexpr static const char IS_LEFT_CHILD = 0x1;

    /**
     * @brief Compare two keys
     * @return -1 if k1 < k2,
     *         0 if k1 == k2,
     *         1 if k1 > k2
     */
    int key_comp_wrapper(const K& k1, const K& k2) const noexcept {
        if (comp(k1, k2)) {
            return -1;
        }
        return comp(k2, k1) ? 1 : 0;
    }

public:
    binary_tree_base() = default;
    
    binary_tree_base(const Compare& comp, const Allocator& allocator) 
        : comp(comp), node_allocator(allocator) { }

    binary_tree_base(const binary_tree_base& other, const Allocator& allocator) 
        : key_of(other.key_of), comp(other.comp), node_allocator(allocator) { }

    binary_tree_base(binary_tree_base&& other, const Allocator& allocator) 
        : key_of(std::move(other.key_of)), comp(std::move(other.comp)), node_allocator(allocator) { }

    virtual ~binary_tree_base() = default;

    /**
     * @brief Get a pointer to the underlying node type
     */
    TreeType* underlying_ptr() noexcept {
        return static_cast<TreeType*>(this);
    }

    NodeType* find(NodeType* root, const K& key) const {
        NodeType* curr = root;
        while (curr != nullptr) {
            int result = this -> key_comp_wrapper(key, key_of(curr -> value));
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

    NodeType* upper_bound(NodeType* root, const K& key) const {
        NodeType* curr = root;
        NodeType* res = nullptr;
        while (curr != nullptr) {
            int result = this -> key_comp_wrapper(key, key_of(curr -> value));
            if (result < 0) {
                res = curr;
                curr = curr -> left_child.get();
            } else {
                curr = curr -> right_child.get();
            }
        }
        return res;
    }

    NodeType* lower_bound(NodeType* root, const K& key) const {
        NodeType* curr = root;
        NodeType* res = nullptr;
        while (curr != nullptr) {
            int result = this -> key_comp_wrapper(key, key_of(curr -> value));
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
            int result = this -> key_comp_wrapper(key, key_of(curr -> value));
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
     * @return unique_ptr_type the tree after combining the two trees
     */
    template<typename Resolver, typename Combinator>
    unique_ptr_type set_operation(unique_ptr_type root1, unique_ptr_type root2, Resolver& resolver,
        Combinator& combinator) {
        if (!root1 || !root2) {
            if (combinator(root1 != nullptr, root2 != nullptr)) {
                return root1 ? std::move(root1) : std::move(root2);
            }
            if (root1) {
                root1.release() -> deep_destroy(node_allocator);
            }
            if (root2) {
                root2.release() -> deep_destroy(node_allocator);
            }
            return nullptr;
        }
        node_type* left1 = root1 -> orphan_left_child();
        node_type* right1 = root1 -> orphan_right_child();
        std::pair<unique_ptr_type, bool> split_result(underlying_ptr() -> split(std::move(root2), std::move(root1), resolver));
        unique_ptr_type split_root = std::move(split_result.first);

        unique_ptr_type result_left(set_operation(unique_ptr_type(left1), std::move(split_root -> left_child), resolver, combinator));
        unique_ptr_type result_right(set_operation(unique_ptr_type(right1), std::move(split_root -> right_child), resolver, combinator));

        if (combinator(true, split_result.second)) {
            return TreeType::join(std::move(result_left), std::move(split_root), std::move(result_right));
        }
        split_root.release() -> destroy(node_allocator);
        return TreeType::join(std::move(result_left), std::move(result_right));
    }

    template<typename Resolver, typename Combinator>
    unique_ptr_type set_operation(unique_ptr_type root1, unique_ptr_type root2, thread_pool_executor& executor,
        Resolver& resolver, Combinator& combinator) {
        if (!root1 || !root2) {
            if (combinator(root1 != nullptr, root2 != nullptr)) {
                return root1 ? std::move(root1) : std::move(root2);
            }
            if (root1) {
                root1.release() -> deep_destroy(node_allocator);
            }
            if (root2) {
                root2.release() -> deep_destroy(node_allocator);
            }
            return nullptr;
        }
        node_type* left1 = root1 -> orphan_left_child();
        node_type* right1 = root1 -> orphan_right_child();
        std::pair<unique_ptr_type, bool> split_result(underlying_ptr() -> split(std::move(root2), std::move(root1), resolver));
        unique_ptr_type split_root = std::move(split_result.first);

        std::future<unique_ptr_type> future;
        bool is_parallel = should_parallelize(left1, split_root -> left_child.get());
        
        unique_ptr_type result_left;
        unique_ptr_type result_right;
        if (is_parallel) {
            auto lambda = [this](unique_ptr_type root1, unique_ptr_type root2,
                thread_pool_executor& executor, Resolver& resolver, Combinator& combinator) {
                return set_operation(std::move(root1), std::move(root2), executor, resolver, combinator);
            };
            task<unique_ptr_type> task(
                lambda, unique_ptr_type(left1), std::move(split_root -> left_child), executor, resolver, combinator);
            future = task.get_future();
            executor.attempt_parallel(std::move(task));
            result_right = set_operation(unique_ptr_type(right1), std::move(split_root -> right_child), executor, resolver, combinator);
            result_left = future.get();
        } else {
            result_left = set_operation(unique_ptr_type(left1), std::move(split_root -> left_child), resolver, combinator);
            result_right = set_operation(unique_ptr_type(right1), std::move(split_root -> right_child), resolver, combinator);
        }

        if (combinator(true, split_result.second)) {
            return TreeType::join(std::move(result_left), std::move(split_root), std::move(result_right));
        }
        split_root.release() -> destroy(node_allocator);
        return TreeType::join(std::move(result_left), std::move(result_right));
    }

public:

    /**
     * @brief A helper function for computing union of two trees
     */
    template<typename Resolver>
    unique_ptr_type union_of(unique_ptr_type root1, unique_ptr_type root2, Resolver& resolver) {
        constexpr auto combinator = std::logical_or();
        return set_operation(std::move(root1), std::move(root2), resolver, combinator);
    }

    template<typename Resolver>
    unique_ptr_type union_of(unique_ptr_type root1, unique_ptr_type root2, thread_pool_executor& executor,
        Resolver& resolver) {
        constexpr auto combinator = std::logical_or();
        return set_operation(std::move(root1), std::move(root2), executor, resolver, combinator);
    }

    /**
     * @brief A helper function for computing intersection of two trees
     */
    template<typename Resolver>
    unique_ptr_type intersection_of(unique_ptr_type root1, unique_ptr_type root2, Resolver& resolver) {
        constexpr auto combinator = std::logical_and();
        return set_operation(std::move(root1), std::move(root2), resolver, combinator);
    }

    template<typename Resolver>
    unique_ptr_type intersection_of(unique_ptr_type root1, unique_ptr_type root2, thread_pool_executor& executor,
        Resolver& resolver) {
        constexpr auto combinator = std::logical_and();

        return set_operation(std::move(root1), std::move(root2), executor, resolver, combinator);
    }
    
    /**
     * @brief Helper method for computing difference of two trees
     */
    unique_ptr_type difference_of(unique_ptr_type root1, unique_ptr_type root2) {
        constexpr auto combinator = [](bool contains1, bool contains2) {
            return contains1 && !contains2;
        };
        chooser<value_type> dummyResolver;
        return set_operation(std::move(root1), std::move(root2), dummyResolver, combinator);
    }

    unique_ptr_type difference_of(unique_ptr_type root1, unique_ptr_type root2, thread_pool_executor& executor) {
        constexpr auto combinator = [](bool contains1, bool contains2) {
            return contains1 && !contains2;
        };
        chooser<value_type> dummyResolver;
        return set_operation(std::move(root1), std::move(root2), executor, dummyResolver, combinator);
    }

    void swap(binary_tree_base& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        std::swap(key_of, other.key_of);
        std::swap(comp, other.comp);
        std::swap(node_allocator, other.node_allocator);
    }

    template<std::forward_iterator ForwardIt>
    bool __is_inorder(ForwardIt begin, ForwardIt end) const noexcept {
        if (begin == end) {
            return true;
        }
        for (ForwardIt it = begin; std::next(it) != end; ++it) {
            ForwardIt successor = std::next(it);
            if (this -> key_comp_wrapper(this -> key_of(*it), this -> key_of(*successor)) > 0) {
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
    bool __is_size_correct(NodeType* node, unsigned expected_size) const noexcept {
        unsigned actual_size = compute_size(node);
        if (actual_size != expected_size) {
            return false;
        }
        return true;
    }
};

}
