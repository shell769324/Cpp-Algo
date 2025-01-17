#pragma once
#include "binary_tree_node.h"
#include "binary_tree_common.h"
#include "binary_tree_iterator.h"
#include "src/thread_pool_executor/thread_pool_executor.h"
#include <functional>
#include <utility>

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
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using node_type = NodeType;
    using node_allocator_type = typename alloc_traits::template rebind_alloc<node_type>;
    using node_alloc_traits = typename alloc_traits::template rebind_traits<node_type>;
    using set_op_return_type = std::pair<node_type*, size_type>;

    KeyOf key_of;
    Compare comp;
    node_allocator_type node_allocator;

    node_type* sentinel;
    node_type* begin_node;
    std::size_t element_count;

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
    binary_tree_base() 
        : sentinel(node_type::construct_sentinel(node_allocator)),
          begin_node(sentinel),
          element_count(0) { }
    
    binary_tree_base(const Compare& comp, const Allocator& allocator) 
        : comp(comp), 
          node_allocator(allocator),
          sentinel(node_type::construct_sentinel(node_allocator)),
          begin_node(sentinel),
          element_count(0) { }

    binary_tree_base(const binary_tree_base& other)
        : key_of(other.key_of),
          comp(other.comp), 
          node_allocator(other.node_allocator),
          sentinel(node_type::construct_sentinel(node_allocator)),
          element_count(other.element_count) {
        if (other.sentinel -> left_child) {
            sentinel -> link_left_child(other.sentinel -> left_child -> deep_clone(node_allocator));
        }
        begin_node = sentinel -> get_leftmost_descendant();
    }

    binary_tree_base(const binary_tree_base& other, const Allocator& allocator) 
        : key_of(other.key_of), 
          comp(other.comp), 
          node_allocator(allocator),
          sentinel(node_type::construct_sentinel(node_allocator)),
          element_count(other.element_count) {
        if (other.sentinel -> left_child) {
            sentinel -> link_left_child(other.sentinel -> left_child -> deep_clone(node_allocator));
        }
        begin_node = sentinel -> get_leftmost_descendant();
    }

    binary_tree_base(binary_tree_base&& other) 
        : key_of(std::move(other.key_of)), 
          comp(std::move(other.comp)), 
          node_allocator(other.node_allocator),
          sentinel(other.sentinel),
          begin_node(other.begin_node),
          element_count(other.element_count) {
        other.sentinel = nullptr;
        other.begin_node = nullptr;
        other.element_count = 0;
    }

    binary_tree_base(binary_tree_base&& other, const Allocator& allocator) 
        : key_of(std::move(other.key_of)), 
          comp(std::move(other.comp)), 
          node_allocator(allocator),
          sentinel(other.sentinel),
          begin_node(other.begin_node),
          element_count(other.element_count) {
        other.sentinel = nullptr;
        other.begin_node = nullptr;
        other.element_count = 0;
    }

    // TODO determine if virtual destructor is needed
    ~binary_tree_base() noexcept {
        // If this tree was moved, its sentinel is null
        if (sentinel == nullptr) {
            return;
        }
        // Destroy and deallocate all nodes
        clear();
        node_alloc_traits::deallocate(node_allocator, sentinel, 1);
    }

    /**
     * @brief Get a pointer to the underlying node type
     */
    TreeType* underlying_ptr() noexcept {
        return static_cast<TreeType*>(this);
    }


protected:
    /**
     * @brief Get the node that would be the parent of the new node when the
     *        given key is inserted
     * 
     * No insertion is performed
     * 
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
    std::pair<NodeType*, char>
    get_insertion_parent(const K& key) const {
        bool result = true;
        NodeType* prev = sentinel;
        NodeType* curr = sentinel -> left_child;
        while (curr != nullptr) {
            prev = curr;
            result = comp(key, key_of(curr -> value));
            curr = result ? curr -> left_child : curr -> right_child;
        }
        NodeType* left_node = prev;
        if (result) {
            if (begin_node == prev) {
                return std::make_pair(prev, IS_LEFT_CHILD);
            }
            left_node = left_node -> prev();
        }
        if (comp(key_of(left_node -> value), key)) {
            return std::make_pair(prev, static_cast<char>(result));
        }
        return std::make_pair(left_node, EXISTS);
    }

    /**
     * @brief Get the node that would be the parent of the new node when the
     *        given key is inserted
     * 
     * No insertion is performed
     * 
     * @param hint an iterator pointing a location that is close to the location of insertion.
     *             If the iterator is a wrong guess, this method will invoke the version without the hint
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
    std::pair<NodeType*, char> get_insertion_parent(const_iterator hint, const K& key) const {
        if (hint == cend()) {
            if (element_count == 0) {
                return std::make_pair(hint.node, IS_LEFT_CHILD);
            } else {
                const_iterator hint_prev = std::prev(hint);
                if (comp(key_of(*hint_prev), key)) {
                    return std::make_pair(hint_prev.node, 0);
                } else {
                    return get_insertion_parent(key);
                }
            }
        } else if (comp(key, key_of(*hint))) {
            if (hint.node == begin_node) {
                return std::make_pair(hint.node, IS_LEFT_CHILD);
            } else {
                const_iterator hint_prev = std::prev(hint);
                if (comp(key_of(*hint_prev), key)) {
                    if (hint.node -> left_child == nullptr) {
                        return std::make_pair(hint.node, IS_LEFT_CHILD);
                    }
                    return std::make_pair(hint_prev.node, 0);
                } else {
                    return get_insertion_parent(key);
                }
            }
        } else if (comp(key_of(*hint), key)) {
            const_iterator hint_next = std::next(hint);
            if (hint_next.node == sentinel) {
                return std::make_pair(hint.node, 0);
            } else {
                if (comp(key, key_of(*hint_next))) {
                    if (hint.node -> right_child == nullptr) {
                        return std::make_pair(hint.node, 0);
                    }
                    return std::make_pair(hint_next.node, IS_LEFT_CHILD);
                } else {
                    return get_insertion_parent(key);
                }
            }
        } else {
            return std::make_pair(hint.node, EXISTS);
        }
    }

    void update_begin_node(node_type* new_node) {
        if (begin_node == sentinel || this -> comp(key_of(new_node -> value), key_of(begin_node -> value))) {
            begin_node = new_node;
        }
    }

public:
    /**
     * @brief Get a copy of the associated allocator
     */
    allocator_type get_allocator() const noexcept {
        return allocator_type(node_allocator);
    }

    /**
     * @brief Test if a tree is empty
     */
    bool empty() const noexcept {
        return element_count == 0;
    }

    /**
     * @brief Get the number of elements in the tree
     */
    std::size_t size() const noexcept {
        return element_count;
    }

    /**
     * @brief Get an iterator to the smallest element
     */
    iterator begin() noexcept {
        return iterator(begin_node);
    }

    /**m
     * @brief Get a constant iterator to the smallest element
     */
    const_iterator begin() const noexcept {
        return const_iterator(begin_node);
    }

    /**
     * @brief Get a constant iterator to the smallest element
     */
    const_iterator cbegin() const noexcept {
        return const_iterator(begin_node);
    }

    /**
     * @brief Get an iterator to one past the greatest element
     */
    iterator end() noexcept {
        return iterator(sentinel);
    }

    /**
     * @brief Get a constant iterator to one past the greatest element
     */
    const_iterator end() const noexcept {
        return const_iterator(sentinel);
    }

    /**
     * @brief Get a constant iterator to one past the greatest element
     */
    const_iterator cend() const noexcept {
        return const_iterator(sentinel);
    }

    /**
     * @brief Get an reverse iterator to the greatest element
     */
    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    /**
     * @brief Get a constant reverse iterator to the greatest element
     */
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    /**
     * @brief Get a constant reverse iterator to the greatest element
     */
    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    /**
     * @brief Get a reverse iterator to one past the smallest element
     */
    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    /**
     * @brief Get a constant reverse iterator to one past the smallest element
     */
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    /**
     * @brief Get a constant reverse iterator to one past the smallest element
     */
    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    /**
     * @brief Remove all elements in this tree
     */
    void clear() noexcept {
        if (sentinel -> left_child) {
            sentinel -> left_child -> deep_destroy(node_allocator);
            sentinel -> left_child = nullptr;
            begin_node = sentinel;
            element_count = 0;
        }
    }

protected:
    NodeType* find_helper(const key_type& key) const {
        NodeType* res = lower_bound_helper(key);
        return res == sentinel || comp(key, key_of(res -> value)) ? sentinel : res;
    }

public:
    /**
     * @brief Get the iterator to an element given a key
     * 
     * @param key the key to look up
     * @return an iterator to the element if it exists,
     *         the end iterator otherwise
     */
    iterator
    find(const key_type& key) {
        return iterator(find_helper(key));
    }

    /**
     * @brief Get the iterator to an element given a key
     * 
     * @param key the key to look up
     * @return a constant iterator to the element if it exists,
     *         the end iterator otherwise
     */
    const_iterator find(const key_type& key) const {
        return const_iterator(find_helper(key));
    }

protected:
    NodeType* upper_bound_helper(const key_type& key) const {
        NodeType* curr = sentinel -> left_child;
        NodeType* res = sentinel;
        while (curr != nullptr) {
            if (comp(key, key_of(curr -> value))) {
                res = curr;
                curr = curr -> left_child;
            } else {
                curr = curr -> right_child;
            }
        }
        return res;
    }

public:
    /**
     * @brief Get the iterator to the smallest element greater than
     *        a given key
     * 
     * @param key the inclusive upper bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    iterator upper_bound(const key_type& key) {
        return iterator(upper_bound_helper(key));
    }

    /**
     * @brief Get the iterator to the smallest element greater than
     *        a given key
     * 
     * @param key the inclusive upper bound
     * @return a const iterator to such element if it exists,
     *         the end iterator otherwise
     */
    const_iterator upper_bound(const key_type& key) const {
        return const_iterator(upper_bound_helper(key));
    }

protected:
    NodeType*
    lower_bound_helper(const key_type& key) const {
        NodeType* curr = sentinel -> left_child;
        NodeType* res = sentinel;
        while (curr != nullptr) {
            if (!comp(key_of(curr -> value), key)) {
                res = curr;
                curr = curr -> left_child;
            } else {
                curr = curr -> right_child;
            }
        }
        return res;
    }

public:
    /**
     * @brief Get the iterator to the smallest element greater than or equal to
     *        a given key
     * 
     * @param key the inclusive lower bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    iterator lower_bound(const key_type& key) {
        return iterator(lower_bound_helper(key));
    }

    /**
     * @brief Get the iterator to the smallest element greater than or equal to
     *        a given key
     * 
     * @param key the inclusive lower bound
     * @return a const iterator to such element if it exists,
     *         the end iterator otherwise
     */
    const_iterator lower_bound(const key_type& key) const {
        return const_iterator(lower_bound_helper(key));
    }


private:
    template<typename Combinator>
    set_op_return_type handle_base_case(node_type* root1, node_type* root2, Combinator& combinator) {
        if (combinator(root1 != nullptr, root2 != nullptr)) {
            return set_op_return_type(root1 ? root1 : root2, 0);
        }
        size_type destroyed = 0;
        if (root1) {
            destroyed += root1 -> deep_destroy_count(node_allocator);
        }
        if (root2) {
            destroyed += root2 -> deep_destroy_count(node_allocator);
        }
        return set_op_return_type(nullptr, destroyed);
    }

    template<typename Combinator>
    set_op_return_type join_split_result(std::pair<node_type*, bool>& split_result,
                                         node_type* result_left,
                                         node_type* result_right,
                                         size_type destroyed,
                                         Combinator& combinator) {
        if (split_result.second) {
            ++destroyed;
        }
        if (combinator(true, split_result.second)) {
            node_type* res = TreeType::join(result_left, split_result.first, result_right);
            return set_op_return_type(res, destroyed);
        }
        split_result.first -> destroy(node_allocator);
        ++destroyed;
        node_type* res = TreeType::join(result_left, result_right);
        return set_op_return_type(res, destroyed);
    }

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
     * @return node_type* the tree after combining the two trees
     */
    template<typename Resolver, typename Combinator>
    set_op_return_type set_operation(node_type* root1, node_type* root2, Resolver& resolver,
        Combinator& combinator) {
        if (!root1 || !root2) {
            return handle_base_case(root1, root2, combinator);
        }
        node_type* left1 = root1 -> orphan_left_child();
        node_type* right1 = root1 -> orphan_right_child();
        std::pair<node_type*, bool> split_result(underlying_ptr() -> split(root2, root1, resolver));
        node_type* split_root = split_result.first;

        auto [result_left, left_destroyed] = 
            set_operation(left1, move(split_root -> left_child), resolver, combinator);
        auto [result_right, right_destroyed] = 
            set_operation(right1, move(split_root -> right_child), resolver, combinator);
        
        size_type destroyed = left_destroyed + right_destroyed;
        
        return join_split_result(split_result, result_left, result_right, destroyed, combinator);
    }

    template<typename Resolver, typename Combinator>
    set_op_return_type set_operation(node_type* root1, node_type* root2, thread_pool_executor& executor,
        Resolver& resolver, Combinator& combinator) {
        if (!root1 || !root2) {
            return handle_base_case(root1, root2, combinator);
        }
        node_type* left1 = root1 -> orphan_left_child();
        node_type* right1 = root1 -> orphan_right_child();
        std::pair<node_type*, bool> split_result(underlying_ptr() -> split(root2, root1, resolver));
        node_type* split_root = split_result.first;

        std::future<set_op_return_type> future;
        bool is_parallel = should_parallelize(left1, split_root -> left_child);
        
        node_type* result_left;
        node_type* result_right;
        size_type destroyed;
        if (is_parallel) {
            auto lambda = [this](node_type* root1, node_type* root2,
                thread_pool_executor& executor, Resolver& resolver, Combinator& combinator) {
                return set_operation(root1, root2, executor, resolver, combinator);
            };
            task<set_op_return_type> task(
                lambda, left1, move(split_root -> left_child), executor, resolver, combinator);
            future = task.get_future();
            executor.attempt_parallel(std::move(task));
            auto [result_right_node, right_destroyed] =
                set_operation(right1, move(split_root -> right_child), executor, resolver, combinator);
            auto [result_left_node, left_destroyed] = future.get();
            destroyed = left_destroyed + right_destroyed;
            result_left = result_left_node;
            result_right = result_right_node;
        } else {
            auto [result_left_node, left_destroyed] = 
                set_operation(left1, move(split_root -> left_child), resolver, combinator);
            auto [result_right_node, right_destroyed] =
                set_operation(right1, move(split_root -> right_child), resolver, combinator);
            destroyed = left_destroyed + right_destroyed;
            result_left = result_left_node;
            result_right = result_right_node;
        }

        return join_split_result(split_result, result_left, result_right, destroyed, combinator);
    }

protected:
    /**
     * @brief A helper function for computing union of two trees
     */
    template<typename Resolver>
    set_op_return_type union_of(node_type* root1, node_type* root2, Resolver& resolver) {
        constexpr auto combinator = std::logical_or();
        return set_operation(root1, root2, resolver, combinator);
    }

    template<typename Resolver>
    set_op_return_type union_of(node_type* root1, node_type* root2, thread_pool_executor& executor,
        Resolver& resolver) {
        constexpr auto combinator = std::logical_or();
        return set_operation(root1, root2, executor, resolver, combinator);
    }

    template<typename Resolver=chooser<value_type> >
    static TreeType union_of_helper(TreeType&& tree1, TreeType&& tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor, Resolver resolver) {
        if (tree1.empty()) {
            return std::move(tree2);
        }
        if (tree2.empty()) {
            return std::move(tree1);
        }
        
        size_type total = tree1.element_count + tree2.element_count;
        node_type* root1 = tree1.sentinel -> orphan_left_child();
        tree1.element_count = 0;
        node_type* root2 = tree2.sentinel -> orphan_left_child();
        tree2.element_count = 0;
        auto [res, destroyed] = executor.has_value() 
            ? tree1.union_of(root1, root2, executor.value().get(), resolver)
            : tree1.union_of(root1, root2, resolver);
        tree1.sentinel -> link_left_child(res);
        tree1.begin_node = tree1.sentinel -> get_leftmost_descendant();
        tree1.element_count = total - destroyed;
        return std::move(tree1);
    }

    /**
     * @brief A helper function for computing intersection of two trees
     */
    template<typename Resolver>
    set_op_return_type intersection_of(node_type* root1, node_type* root2, Resolver& resolver) {
        constexpr auto combinator = std::logical_and();
        return set_operation(root1, root2, resolver, combinator);
    }

    template<typename Resolver>
    set_op_return_type intersection_of(node_type* root1, node_type* root2, thread_pool_executor& executor,
        Resolver& resolver) {
        constexpr auto combinator = std::logical_and();

        return set_operation(root1, root2, executor, resolver, combinator);
    }

    template<typename Resolver=chooser<value_type> >
    static TreeType intersection_of_helper(TreeType&& tree1, TreeType&& tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor, Resolver resolver) 
        requires is_resolver<value_type, Resolver> {
        if (tree1.empty()) {
            return std::move(tree1);
        }
        if (tree2.empty()) {
            return std::move(tree2);
        }
        
        size_type total = tree1.element_count + tree2.element_count;
        node_type* root1 = tree1.sentinel -> orphan_left_child();
        tree1.element_count = 0;
        node_type* root2 = tree2.sentinel -> orphan_left_child();
        tree2.element_count = 0;

        auto [res, destroyed] = executor.has_value() 
            ? tree1.intersection_of(root1, root2, executor.value().get(), resolver)
            : tree1.intersection_of(root1, root2, resolver);
        tree1.sentinel -> nullable_link_left_child(res);
        tree1.begin_node = tree1.sentinel -> get_leftmost_descendant();
        tree1.element_count = total - destroyed;
        return std::move(tree1);
    }
    
    /**
     * @brief Helper method for computing difference of two trees
     */
    set_op_return_type difference_of(node_type* root1, node_type* root2) {
        constexpr auto combinator = [](bool contains1, bool contains2) {
            return contains1 && !contains2;
        };
        chooser<value_type> dummyResolver;
        return set_operation(root1, root2, dummyResolver, combinator);
    }

    set_op_return_type difference_of(node_type* root1, node_type* root2, thread_pool_executor& executor) {
        constexpr auto combinator = [](bool contains1, bool contains2) {
            return contains1 && !contains2;
        };
        chooser<value_type> dummyResolver;
        return set_operation(root1, root2, executor, dummyResolver, combinator);
    }

    static TreeType difference_of_helper(TreeType&& tree1, TreeType&& tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor) {
        if (tree1.empty() || tree2.empty()) {
            return std::move(tree1);
        }

        size_type total = tree1.element_count + tree2.element_count;
        node_type* root1 = tree1.sentinel -> orphan_left_child();
        tree1.element_count = 0;
        node_type* root2 = tree2.sentinel -> orphan_left_child();
        tree2.element_count = 0;

        auto [res, destroyed] = executor.has_value()
            ? tree1.difference_of(root1, root2, executor.value().get())
            : tree1.difference_of(root1, root2);
        tree1.sentinel -> nullable_link_left_child(res);
        tree1.begin_node = tree1.sentinel -> get_leftmost_descendant();
        tree1.element_count = total - destroyed;
        return std::move(tree1);
    }

public:
    void swap(binary_tree_base& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        std::swap(key_of, other.key_of);
        std::swap(comp, other.comp);
        std::swap(node_allocator, other.node_allocator);
        std::swap(sentinel, other.sentinel);
        std::swap(element_count, other.element_count);
        std::swap(begin_node, other.begin_node);
    }

    template<std::forward_iterator ForwardIt>
    bool __is_inorder(ForwardIt begin, ForwardIt end) const noexcept {
        if (begin == end) {
            return true;
        }
        for (ForwardIt it = begin; std::next(it) != end; ++it) {
            ForwardIt successor = std::next(it);
            if (!comp(this -> key_of(*it), this -> key_of(*successor))) {
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
        return 1 + compute_size(node -> left_child) + compute_size(node -> right_child);
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

    bool __is_begin_node_correct() const noexcept {
        return begin_node == sentinel -> get_leftmost_descendant();
    }
};

}
