#pragma once
#include "binary_tree_node.h"
#include "binary_tree_common.h"

namespace algo {
template<typename, typename>
struct rebinder {};

template<typename A, template<typename> typename C, typename B>
struct rebinder<A, C<B>> {
    using type = C<A>;
};

// Get the type of a container with its value type swapped to T
template<typename T, typename Node>
using non_constify = rebinder<std::remove_const_t<T>, Node>::type;

template<typename T, typename Node>
using constify = rebinder<std::add_const_t<T>, Node>::type;

/**
 * @brief Bidirectional iterator for binary tree
 * 
 * @tparam T the type of the value inside the iterator
 */
template <typename Node>
class binary_tree_iterator {
    using T = typename Node::value_type;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = std::ptrdiff_t;

private:
    using non_const_T = std::remove_const_t<T>;

    // Note that any data structure in this package only accepts
    // cv-unqualified data type so the only occurrence of the type T
    // in the template being const-qualified is const_iterator

    // The tree node's data type T is always non-const qualified
    // The reason to store the non-const node is to faciliate easy conversion from
    // non-const iterator to const iterator
    using node_type = non_constify<T, Node>;
    using const_node_type = constify<T, Node>;
    node_type* node;

public:
    /**
     * @brief Construct a tree iterator from a binary tree node
     * 
     * @param node the node to start iterating from
     */
    binary_tree_iterator(node_type* node = nullptr) : node(node) { }

    /**
     * @brief Construct a copy of another tree iterator
     * 
     * Copy constructor
     * 
     * @param other the iterator to copy from
     */
    binary_tree_iterator(const binary_tree_iterator& other) = default;

    /**
     * @brief Construct a copy of another tree iterator by moving it
     * 
     * Move constructor
     * 
     * @param other the iterator to move from
     */
    binary_tree_iterator(binary_tree_iterator&& other) = default;

    /**
     * @brief Copy assign from another tree iterator
     */
    binary_tree_iterator& operator=(const binary_tree_iterator& other) = default;

    /**
     * @brief Move assign from another tree iterator
     */
    binary_tree_iterator& operator=(binary_tree_iterator&& other) = default;

private:
    /**
     * @brief Construct a tree iterator from its const counterpart
     * 
     * This constructor is private to disallow users from breaking constness of iterator
     * It is also declared explicit to
     * 1) prevent implicitly calling this dangerous conversion
     * 2) resolve ambiguity of the equality operator overload. Note that if this is not explicit,
     *    the equality operator overloads for the non-const version and the const version will be
     *    equally matching. Making this constructor explicit breaks the tie.
     * 
     * @param other the const iterator to copy from
     */
    explicit binary_tree_iterator(const binary_tree_iterator<const_node_type>& other)
        requires (!std::is_const_v<T>) : node(other.node) { }

public:
    /**
     * @brief Construct a const tree iterator from non-const counterpart
     * 
     * @param other a non-const tree iterator
     */
    binary_tree_iterator(const binary_tree_iterator<node_type>& other)
        requires std::is_const_v<T> : node(other.node) { }

    /**
     * @brief Get a reference to current value
     */
    reference operator*() const noexcept {
        return node -> value;
    }

    /**
     * @brief Get a pointer to current value
     */
    pointer operator->() const noexcept {
        return &(node -> value);
    }

    /**
     * @brief Increment this iterator
     * 
     * @return a copy of this iterator after incrementing
     */
    binary_tree_iterator& operator++() noexcept {
        node = node -> next();
        return *this;
    }

    /**
     * @brief Increment this iterator
     * 
     * @return a copy of this iterator before incrementing
     */
    binary_tree_iterator operator++(int) noexcept {
        binary_tree_iterator tmp = *this;
        node = node -> next();
        return tmp;
    }
    
    /**
     * @brief Decrement this iterator
     * 
     * @return a copy of this iterator after decrementing 
     */
    binary_tree_iterator& operator--() noexcept {
        node = node -> prev();
        return *this;
    }

    /**
     * @brief Decrement this iterator
     * 
     * @return a copy of this iterator before decrementing
     */
    binary_tree_iterator operator--(int) noexcept {
        binary_tree_iterator tmp = *this;
        node = node -> prev();
        return tmp;
    }

    /**
     * @brief Check equality of two iterators
     * 
     * @param it1 the first iterator
     * @param it2 the second iterator
     * @return true if they are equal, false otherwise
     */
    friend bool operator==(const binary_tree_iterator& it1, const binary_tree_iterator& it2) noexcept {
        return it1.node == it2.node;
    }

    // Ideally the value type V should be T but cpp doesn't allow partially
    // specialized template friend class
    template <typename K, typename V, typename KeyOf, typename NodeType, typename TreeType, typename Compare, typename Allocator>
    requires binary_tree_definable<K, V, KeyOf, Compare, Allocator>
    friend class binary_tree_base;

    template <typename K, typename V, typename KeyOf, typename Comparator, typename Allocator>
    requires binary_tree_definable<K, V, KeyOf, Comparator, Allocator>
    friend class avl_tree;

    template <typename K, typename V, typename KeyOf, typename Comparator, typename Allocator>
    requires binary_tree_definable<K, V, KeyOf, Comparator, Allocator>
    friend class red_black_tree;

    // const and non-const iterator of the same base type are friends
    friend std::conditional<std::is_const_v<T>, binary_tree_iterator<node_type>, void>::type;
    friend std::conditional<(!std::is_const_v<T>), binary_tree_iterator<const_node_type>, void>::type;
};
}
