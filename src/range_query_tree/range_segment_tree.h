#pragma once

#include <optional>
#include <stdexcept>
#include <concepts>
#include <algorithm>
#include <climits>
#include <iterator>
#include <memory>
#include "src/common.h"
#include "src/range_query_tree/segment_tree_common.h"


namespace algo {

/**
 * @brief Data structure that supports efficient update, query and range update
 * 
 * @tparam T the type of value in the data structure
 * @tparam Operator the type of associated binary operator. Must be associative
 * @tparam RepeatOperator the type of operator that computes the result of repeatedly applying
 *         the operator over the same value a number of times. For example, if the operator is plus,
 *         the repeat operator would be multiply
 * @tparam Allocator the allocator to construct and destroy elements
 */
template <typename T, typename Operator, typename RepeatOperator, typename Allocator = std::allocator<T> >
requires range_segment_tree_definable<T, Operator, RepeatOperator, Allocator>
class range_segment_tree {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using operator_type = Operator;
    using repeat_operator_type = RepeatOperator;
    
private:
    using alloc_traits = std::allocator_traits<Allocator>;
    using cleaner_type = std::unique_ptr<T, deleter<T, allocator_type> >;
    using subtree_type = std::tuple<T*, std::size_t, std::size_t>;
    using subtree_allocator_type = typename alloc_traits::template rebind_alloc<subtree_type>;
    using subtree_unique_ptr = std::unique_ptr<subtree_type[], deleter<subtree_type, subtree_allocator_type> >;
    using bool_allocator_type = typename alloc_traits::template rebind_alloc<bool>;
    using bool_alloc_traits = typename alloc_traits::template rebind_traits<bool>;
    using initializer_type = segment_tree_initializer<T, Operator, Allocator>;

    std::size_t length;
    pointer data;
    pointer lazy_data;
    // If lazy_flags[i] is true, data[i] is updated but its children need to be updated by applying
    // the repeat operator with lazy_data[i]
    bool* lazy_flags;
    Operator op;
    RepeatOperator rop;
    Allocator allocator;
    subtree_allocator_type subtree_allocator;
    bool_allocator_type bool_allocator;

    std::size_t get_data_length() const noexcept {
        return length * 2 - 1;
    }

    void allocate_data() {
        std::size_t data_length = get_data_length();
        data = alloc_traits::allocate(allocator, data_length);
        cleaner_type data_cleaner(data, deleter<T, Allocator>(data_length, allocator));
        lazy_data = alloc_traits::allocate(allocator, data_length);
        cleaner_type lazy_data_cleaner(data, deleter<T, Allocator>(data_length, allocator));
        lazy_flags = bool_alloc_traits::allocate(bool_allocator, data_length);
        std::fill(lazy_flags, lazy_flags + data_length, false);
        data_cleaner.release();
        lazy_data_cleaner.release();
    }

    void deallocate_data() {
        std::size_t data_length = get_data_length();
        alloc_traits::deallocate(allocator, data, data_length);
        alloc_traits::deallocate(allocator, lazy_data, data_length);
        bool_alloc_traits::deallocate(bool_allocator, lazy_flags, data_length);
    }

    template<typename IsIterator, typename... Args>
    void initialize_helper(Args&&... args) {
        allocate_data();
        initializer_type initializer(length, data, op, allocator);
        try {
            initializer.template build<IsIterator>(0, length, data, std::forward<Args>(args)...);
        } catch (...) {
            deallocate_data();
            throw;
        }
    }
    
public:
    /**
     * @brief Construct a new range segment tree object with length, an optional operator, an optional repeat operator,
     *        an optional Allocator
     * 
     * The elements in the tree will be default constructed
     * 
     * @param length the fixed length of the range segment tree
     * @param op the associated binary operator
     * @param rop the associated repeat operator
     * @param allocator the allocator to construct and destroy elements
     */
    range_segment_tree(std::size_t length, const Operator& op = Operator(), 
                       const RepeatOperator& rop = RepeatOperator(), const Allocator& allocator = Allocator())
        requires std::default_initializable<T> 
        : length(length), op(op), rop(rop), allocator(allocator), subtree_allocator(allocator),
          bool_allocator(allocator) {
        initialize_helper<std::false_type>();
    }

    /**
     * @brief Construct a new range segment tree object with length, an optional operator, an optional repeat operator,
     *        an optional Allocator
     * 
     * The elements in the tree will be default constructed
     * 
     * @param length the fixed length of the range segment tree
     * @param value the value to initialize the tree
     * @param op the associated binary operator
     * @param rop the associated repeat operator
     * @param allocator the allocator to construct and destroy elements
     */
    range_segment_tree(std::size_t length, const_reference value, const Operator& op=Operator(), 
                       const RepeatOperator& rop = RepeatOperator(), const Allocator& allocator = Allocator()) 
        requires std::copy_constructible<T>
        : length(length), op(op), rop(rop), allocator(allocator), subtree_allocator(allocator),
          bool_allocator(allocator) {
        initialize_helper<std::false_type>(value);
    }

    /**
     * @brief Construct a new range segment tree from a range
     * 
     * The range segment tree will have the same content as the range defined
     * by the two iterators
     * 
     * @tparam ForwardIt the type of the forward iterator
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @param op the associative binary operator
     * @param rop the associated repeat operator
     * @param allocator the allocator to construct and destroy elements
     */
    template<std::forward_iterator ForwardIt>
    range_segment_tree(ForwardIt first, ForwardIt last, const Operator& op = Operator(), 
                       const RepeatOperator& rop = RepeatOperator(), const Allocator& allocator = Allocator()) 
        requires std::copy_constructible<T>
        : length(std::distance(first, last)), op(op), rop(rop), allocator(allocator), 
          subtree_allocator(allocator), bool_allocator(allocator) {
        initialize_helper<std::true_type>(first);
    }

    /**
     * @brief Copy constructor
     * 
     * @param other the range segment tree to copy from
     */
    range_segment_tree(const range_segment_tree& other) requires std::copy_constructible<T>
        : length(other.length), op(other.op), rop(other.rop), allocator(other.allocator), 
          subtree_allocator(other.subtree_allocator), bool_allocator(other.bool_allocator) {
        allocate_data();
        std::size_t data_length = get_data_length();
        std::copy(other.lazy_flags, other.lazy_flags + data_length, lazy_flags);
        try {
            uninitialized_copy(other.data, other.data + data_length, data, allocator);
        } catch (...) {
            deallocate_data();
            throw;
        }
        std::size_t index = 0;
        try {
            for (; index < data_length; ++index) {
                if (lazy_flags[index]) {
                    alloc_traits::construct(allocator, lazy_data + index, other.lazy_data[index]);
                }
            }
        } catch (...) {
            for (std::size_t i = 0; i < index; ++i) {
                if (lazy_flags[i]) {
                    alloc_traits::destroy(allocator, lazy_data + i);
                }
            }
            destroy(data, data + data_length, allocator);
            deallocate_data();
        }
    }

    /**
     * @brief Move constructor
     * 
     * The other tree will have an unspecified state after this operation
     * 
     * @param other the ranget segment tree to copy from by move
     */
    range_segment_tree(range_segment_tree&& other) noexcept :
        length(0),
        data(nullptr),
        op(other.op),
        rop(other.rop) {
        swap(other);
    }

    /**
     * @brief Destructor
     * 
     * Destroy all data in the segment tree and free memory allocated
     */
    ~range_segment_tree() {
        if (data == nullptr) {
            return;
        }
        std::size_t data_length = get_data_length();
        destroy(data, data + data_length, allocator);
        for (std::size_t i = 0; i < data_length; ++i) {
            if (lazy_flags[i]) {
                alloc_traits::destroy(allocator, lazy_data + i);
            }
        }
        deallocate_data();
    }

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another range segment tree into this one
     * 
     * Strong exception guarantee
     * 
     * @param other the segment tree to copy from
     * @return a reference to itself
     */
    range_segment_tree& operator=(const range_segment_tree& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }

        range_segment_tree tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Move assignment operator
     * 
     * Move all elements from another range segment tree into this one. The given
     * tree tree will be left in a valid yet unspecified state
     * 
     * @param other the tree tree to move from
     * @return a reference to itself
     */
    range_segment_tree& operator=(range_segment_tree&& other) noexcept {
        swap(other);
        return *this;
    }


private:
    void write_lazy_data(std::size_t curr, const_reference val) {
        if (lazy_flags[curr]) {
            lazy_data[curr] = val;
        } else {
            alloc_traits::construct(allocator, lazy_data + curr, val);
            lazy_flags[curr] = true;
        }
    }

    void push(T* root, std::size_t left, std::size_t right) {
        std::size_t curr = root - data;
        if (!lazy_flags[curr]) {
            return;
        }
        std::size_t left_index = curr + 1;
        std::size_t mid = (right - left) / 2 + left;
        std::size_t right_index = curr + 2 * (mid - left);
        data[left_index] = rop(mid - left, lazy_data[curr]);
        data[right_index] = rop(right - mid, lazy_data[curr]);

        write_lazy_data(left_index, lazy_data[curr]);
        write_lazy_data(right_index, lazy_data[curr]);

        alloc_traits::destroy(allocator, lazy_data + curr);
        lazy_flags[curr] = false;
    }

    void push_all(std::size_t curr, std::size_t left, std::size_t right) {
        if (!lazy_flags[curr] || left + 1 == right) {
            return;
        }
        
        std::size_t left_index = curr + 1;
        std::size_t mid = (right - left) / 2 + left;
        std::size_t right_index = curr + 2 * (mid - left);
        data[left_index] = rop(mid - left, lazy_data[curr]);
        data[right_index] = rop(right - mid, lazy_data[curr]);

        write_lazy_data(left_index, lazy_data[curr]);
        write_lazy_data(right_index, lazy_data[curr]);

        alloc_traits::destroy(allocator, lazy_data + curr);
        lazy_flags[curr] = false;
        push_all(left_index, left, mid);
        push_all(right_index, mid, right);
    }

    void update_helper(std::size_t first, std::size_t last, const_reference val, T* curr, std::size_t left, std::size_t right) {
        if (left == first && right == last) {
            *curr = rop(right - left, val);
            write_lazy_data(curr - data, val);
            return;
        }
        push(curr, left, right);
        std::size_t mid = (right - left) / 2 + left;
        if (last <= mid) {
            update_helper(first, last, val, curr + 1, left, mid);
        } else if (mid <= first) {
            update_helper(first, last, val, curr + 2 * (mid - left), mid, right);
        } else {
            update_helper(first, mid, val, curr + 1, left, mid);
            update_helper(mid, last, val, curr + 2 * (mid - left), mid, right);
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
     * Runs in log(n) time where n is the size of the range segment tree
     * 
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @return the result of applying op over the range
     */
    T query(std::size_t first, std::size_t last) {
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
     * Runs in log(n) time where n is the size of the range segment tree
     * 
     * @param pos the index of the value to update
     * @param val the new value to copy
     */
    void update(std::size_t pos, const_reference val) requires std::copy_constructible<T> {
        return update_helper(pos, pos + 1, val, data, 0, length);
    }

    /**
     * @brief Update the value at a given range to a new value
     * 
     * Runs in log(n) time where n is the size of the range segment tree
     * 
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @param val the new value to copy
     */
    void update(std::size_t first, std::size_t last, const_reference val) requires std::copy_constructible<T> {
        return update_helper(first, last, val, data, 0, length);
    }

    void swap(range_segment_tree& other) noexcept {
        std::swap(length, other.length);
        std::swap(data, other.data);
        std::swap(lazy_data, other.lazy_data);
        std::swap(lazy_flags, other.lazy_flags);
        std::swap(op, other.op);
        std::swap(allocator, other.allocator);
        std::swap(subtree_allocator, other.subtree_allocator);
        std::swap(bool_allocator, other.bool_allocator);
    }

    /**
     * @brief Get the associated allocator
     */
    Allocator get_allocator() {
        return allocator;
    }

    /**
     * @brief Check equality of two range segment trees
     * 
     * @param tree1 the first segment tree
     * @param tree2 the second segment tree
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const range_segment_tree& tree1, const range_segment_tree& tree2) requires equality_comparable<value_type> {
        if (tree1.size() != tree2.size()) {
            return false;
        }
        (const_cast<range_segment_tree&>(tree1)).push_all(0, 0, tree1.length);
        (const_cast<range_segment_tree&>(tree2)).push_all(0, 0, tree2.length);
        std::size_t data_length = tree1.get_data_length();
        for (const_pointer it1 = tree1.data + 1, it2 = tree2.data + 1; 
             it1 != tree1.data + data_length && it2 != tree2.data + data_length; ++it1, ++it2) {
            if (!(*it1 == *it2)) {
                return false;
            }
        }
        return true;
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
        push_all(0, 0, length);
        return __is_valid_helper(0, length, data);
    }

    value_type* __get_data() {
        return data;
    }

    value_type* __get_lazy_data() {
        return lazy_data;
    }

    bool* __get_lazy_flags() {
        return lazy_flags;
    }

    /**
     * @brief Get the associated subtree allocator
     */
    subtree_allocator_type __get_subtree_allocator() {
        return subtree_allocator;
    }

    friend T query_helper<range_segment_tree, T>(const range_segment_tree& tree, std::size_t first, std::size_t last,
                                                 const T* curr, std::size_t left, std::size_t right);
    friend subtree_unique_ptr collect_subtrees<range_segment_tree>(range_segment_tree& tree, std::size_t first, std::size_t last);

    friend void collect_subtrees_helper<range_segment_tree, T>(
        range_segment_tree& tree, std::tuple<T*, std::size_t, std::size_t>*& fill_pos, 
        T* curr, std::size_t left, std::size_t right, std::size_t first, std::size_t last) noexcept;

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
