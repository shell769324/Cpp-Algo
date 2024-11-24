#pragma once

#include <optional>
#include <stdexcept>
#include <concepts>
#include <algorithm>
#include <iterator>
#include <memory>
#include "src/common.h"
#include "src/allocator_aware_algorithms.h"
#include "src/range_query_tree/segment_tree_common.h"


namespace algo {
template <typename T, typename Operator, typename Allocator>
requires segment_tree_definable<T, Operator, Allocator>
class segment_tree {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using operator_type = Operator;
    
private:
    using alloc_traits = std::allocator_traits<Allocator>;
    using cleaner_type = std::unique_ptr<T, deleter<T, allocator_type> >;
    using subtree_type = std::tuple<T*, std::size_t, std::size_t>;
    using subtree_allocator_type = typename alloc_traits::rebind_alloc<subtree_type>;
    using subtree_unique_ptr = std::unique_ptr<subtree_type[], deleter<subtree_type, subtree_allocator_type> >;
    using initializer_type = segment_tree_initializer<T, Operator, Allocator>;

    std::size_t length;
    pointer data;
    Operator op;
    allocator_type allocator;
    subtree_allocator_type subtree_allocator;

    std::size_t get_data_length() const noexcept {
        return length * 2 - 1;
    }
    
public:
    /**
     * @brief Construct a new segment tree object with length, an optional operator and an optional Allocator
     * 
     * The elements in the tree will be default constructed
     * 
     * @param length the fixed length of the segment tree
     * @param op the associated binary operator
     * @param allocator the allocator to construct and destroy elements
     */
    segment_tree(std::size_t length, const Operator& op = Operator(), const Allocator& allocator=Allocator())
        requires std::default_initializable<T> 
        : length(length), op(op), allocator(allocator), subtree_allocator(allocator) {
        initializer_type initializer(length, data, this -> op, this -> allocator);
        initializer.repeat_initialize_helper();
    }

    /**
     * @brief Construct a new segment tree object with length, a value to copy, an optional operator and an optional Allocator
     * 
     * The elements in the tree will be copies of the given value
     * 
     * @param length the fixed length of the segment tree
     * @param value the value to copy
     * @param op the associated binary operator
     * @param allocator the allocator to construct and destroy elements
     */
    segment_tree(std::size_t length, const_reference value, const Operator& op = Operator(), const Allocator& allocator=Allocator()) 
        requires std::copy_constructible<T>
        : length(length), op(op), allocator(allocator), subtree_allocator(allocator) {
        initializer_type initializer(length, data, this -> op, this -> allocator);
        initializer.repeat_initialize_helper(value);
    }


    /**
     * @brief Construct a new segment tree from a range
     * 
     * The segment tree will have the same content as the range defined
     * by the two iterators
     * 
     * @tparam ForwardIt the type of the forward iterator
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @param op the associative binary operator
     */
    template<std::forward_iterator ForwardIt>
    segment_tree(ForwardIt first, ForwardIt last, const Operator& op = Operator(), const Allocator& allocator = Allocator()) 
        requires std::copy_constructible<T>
        : length(std::distance(first, last)), op(op), allocator(allocator), subtree_allocator(allocator) {
        initializer_type initializer(length, data, this -> op, this -> allocator);
        cleaner_type cleaner = initializer.allocate_data();
        initializer.template build<std::true_type>(0, length, data, first);
        cleaner.release();
    }

    /**
     * @brief Copy constructor
     * 
     * @param other the segment tree to copy from
     */
    segment_tree(const segment_tree& other) requires std::copy_constructible<T>
        : length(other.length), op(other.op), allocator(other.allocator), subtree_allocator(other.subtree_allocator) {
        segment_tree_initializer initializer(length, data, op, this -> allocator);
        cleaner_type cleaner = initializer.allocate_data();
        uninitialized_copy(other.data, other.data + get_data_length(), data, allocator);
        cleaner.release();
    }

    /**
     * @brief Move constructor
     * 
     * The other tree will have an unspecified state after this operation
     * 
     * @param other the segment tree to copy from
     */
    segment_tree(segment_tree&& other) noexcept :
        length(0),
        data(nullptr),
        op(other.op) {
        swap(other);
    }

    /**
     * @brief Destructor
     * 
     * Destroy all data in the segment tree and free memory allocated
     */
    ~segment_tree() {
        if (data == nullptr) {
            return;
        }
        destroy(data, data + get_data_length(), allocator);
        alloc_traits::deallocate(allocator, data, get_data_length());
    }

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another segment tree into this one
     * 
     * Strong exception guarantee
     * 
     * @param other the segment tree to copy from
     * @return a reference to itself
     */
    segment_tree& operator=(const segment_tree& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }

        segment_tree tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Move assignment operator
     * 
     * Move all elements from another segment tree into this one. The given
     * tree tree will be left in a valid yet unspecified state
     * 
     * @param other the tree tree to move from
     * @return a reference to itself
     */
    segment_tree& operator=(segment_tree&& other) noexcept {
        swap(other);
        return *this;
    }


private:

    /**
     * @brief Update the value at index pos
     * 
     * Runs in log(length) time
     * 
     * @pre [left, right) contains pos
     * @param pos the index of the value to update
     * @param curr the root of the current subtree
     * @param left the begin of the range that represents the current subtree
     * @param right the end of the range that represents the current subtree
     * @param val the value to update to
     */
    void update_helper(std::size_t pos, pointer curr, std::size_t left, std::size_t right, T&& val) {
        if (left == pos && left + 1 == right) {
            *curr = std::move(val);
            return;
        }
        std::size_t mid = (right - left) / 2 + left;
        if (pos < mid) {
            update_helper(pos, curr + 1, left, mid, std::move(val));
        } else {
            update_helper(pos, curr + 2 * (mid - left), mid, right, std::move(val));
        }
        *curr = op(curr[1], curr[2 * (mid - left)]);
    }

public:
    /**
     * @brief Get the size of this tree
     * 
     * @return std::size_t the number of elements this tree has
     */
    std::size_t size() const noexcept {
        return length;
    }

    /**
     * @brief Compute the result of applying op over the range [first, last)
     * 
     * Runs in log(n) time where n is the size of the segment tree
     * 
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @return the result of applying op over the range
     */
    T query(std::size_t first, std::size_t last) const {
        if (first == last) {
            throw std::invalid_argument("Cannot query empty range");
        }
        return query_helper(*this, first, last, data, 0, length);
    }

    /**
     * @brief Get an optional index of the exclusive end of the smallest prefix that satisfies the given condition in a range
     * 
     * Runs in log(n) time where n is the size of the segment tree
     * 
     * @pre decider must meet the requirement that decider(op(data[:i])) implies decider(op(data[:j])) where i < j.
     *      Most common operations such as addition, min and max all satisfy this requirement.
     * @tparam F the type of the predicate for prefix result.
     * @param decider the predicate for prefix result
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @return the index if such prefix of the given range exists, or else an empty optional
     */
    template<typename F>
    requires std::regular_invocable<F, T> && std::convertible_to<std::decay_t<std::invoke_result_t<F, T> >, bool>
    std::optional<std::size_t> prefix_search(F decider, std::size_t first, std::size_t last) {
        return algo::prefix_search(*this, decider, first, last);
    }

    /**
     * @brief Get the index of the inclusive start of the smallest suffix that satisfies the given condition in a range
     * 
     * Runs in log(n) time where n is the size of the segment tree
     * 
     * @pre decider must meet the requirement that decider(op(data[i:])) implies decider(op(data[j:])) where i > j
     *      Most common operations such as addition, min and max all satisfy this requirement.
     * @tparam F the type of the predicate for suffix result
     * @param decider the predicate for suffix result
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @return the index if such suffix of the given range exists, or else an empty optional
     */
    template<typename F>
    requires std::regular_invocable<F, T> && std::convertible_to<std::decay_t<std::invoke_result_t<F, T> >, bool>
    std::optional<std::size_t> suffix_search(F decider, std::size_t first, std::size_t last) {
        return algo::suffix_search(*this, decider, first, last);
    }

    /**
     * @brief Update the value at a given position to a new value
     * 
     * Runs in log(n) time where n is the size of the segment tree
     * 
     * @param pos the index of the value to update
     * @param val the new value to copy
     */
    void update(std::size_t pos, const_reference val) requires std::copy_constructible<T> {
        value_holder<value_type> holder;
        // We have to resort to union type so that we can fulfill the promise of constructing all
        // values through the allocator
        alloc_traits::construct(allocator, std::addressof(holder.val), val);
        update_helper(pos, data, 0, length, std::move(holder.val));
        alloc_traits::destroy(allocator, std::addressof(holder.val));
    }

    /**
     * @brief Update the value at a given position to a new value
     * 
     * @param pos the index of the value to update
     * @param val the new value to copy by move
     */
    void update(std::size_t pos, T&& val) requires std::move_constructible<T> {
        return update_helper(pos, data, 0, length, std::move(val));
    }

    void swap(segment_tree& other) noexcept(std::is_nothrow_swappable_v<Operator>) {
        std::swap(length, other.length);
        std::swap(data, other.data);
        std::swap(op, other.op);
        std::swap(allocator, other.allocator);
        std::swap(subtree_allocator, other.subtree_allocator);
    }

    /**
     * @brief Check equality of two segment trees
     * 
     * @param tree1 the first segment tree
     * @param tree2 the second segment tree
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const segment_tree& tree1, const segment_tree& tree2) requires equality_comparable<value_type> {
        if (tree1.size() != tree2.size()) {
            return false;
        }
        std::size_t data_length = tree1.get_data_length();
        for (const_pointer it1 = tree1.data + 1, it2 = tree2.data + 1; 
             it1 != tree1.data + data_length && it2 != tree2.data + data_length; ++it1, ++it2) {
            if (!(*it1 == *it2)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Get the associated allocator
     */
    Allocator get_allocator() {
        return allocator;
    }

    // For testing purpose
private:
    bool __is_valid_helper(std::size_t left, std::size_t right, const T* curr) noexcept {
        if (right - left == 1) {
            return true;
        }
        std::size_t mid = (right - left) / 2 + left;
        // Destroying elements should be owned by the recursive call
        bool result = __is_valid_helper(left, mid, curr + 1);
        // 2 * (mid - left) because that the subtree rooted at curr + 1 will use a subarray of size 2 * (mid - left) - 1
        result = result && __is_valid_helper(mid, right, curr + 2 * (mid - left));
        T expected = op(curr[1], curr[2 * (mid - left)]);
        return result && (expected == *curr);
    }

public:
    bool __is_valid() noexcept requires equality_comparable<value_type> {
        return __is_valid_helper(0, length, data);
    }

    /**
     * @brief Get the associated subtree allocator
     */
    subtree_allocator_type __get_subtree_allocator() {
        return subtree_allocator;
    }

    friend T query_helper<segment_tree, T>(const segment_tree& tree, std::size_t first, std::size_t last, const T* curr, std::size_t left, std::size_t right);
    friend subtree_unique_ptr collect_subtrees<segment_tree>(segment_tree& tree, std::size_t first, std::size_t last);

    template<typename F, typename U, typename Tree>
    friend std::size_t prefix_search_helper(Tree& tree, U* curr, std::size_t left, std::size_t right, std::optional<U> acc, F& decider);

    template<typename Tree, typename F>
    friend std::optional<std::size_t> prefix_search(Tree& tree, F& decider, std::size_t first, std::size_t last);

    template<typename F, typename U, typename Tree>
    friend std::size_t suffix_search_helper(Tree& tree, U* curr, std::size_t left, std::size_t right, std::optional<U> acc, F& decider);

    template<typename Tree, typename F>
    friend std::optional<std::size_t> suffix_search(Tree& tree, F& decider, std::size_t first, std::size_t last);
};
}
