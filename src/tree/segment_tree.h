#pragma once

#include <optional>
#include <stdexcept>
#include <concepts>
#include <algorithm>
#include <iterator>
#include <memory>
#include "src/common.h"
#include "src/vector.h"
#include "src/allocator_aware_algorithms.h"


namespace algo {

class destroy_exception : public exception {
    const exception& cause;
public:
    destroy_exception(const exception other) noexcept : cause(other) { }
}

template <typename T, typename Operator, typename Allocator = std::allocator<T> >
requires std::regular_invocable<Operator, T, T> && std::same_as<T, std::decay_t<std::invoke_result_t<Operator, T, T> > > && std::same_as<T, typename Allocator::value_type>
class segment_tree {
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    
private:
    using alloc_traits = std::allocator_traits<Allocator>;

    std::size_t length;
    pointer data;
    Operator op;
    Allocator allocator;

    template<typename... Args>
    requires !singleton_pack_decayable_to<pointer, Args...>
    T build(std::size_t left, std::size_t right, std::size_t curr, Args&& args...) {
        if (right - left == 1) {
            alloc_traits.construct(allocator, data + curr, std::forward<Args>(args)...);
            return data[curr];
        }
        std::size_t mid = (right - left) / 2 + left;
        std::size_t destroy_end = left;
        try {
            T left_val = build(left, mid, curr + 1, value);
            destroy_end = mid;
            T right_val = build(mid, right, curr + 2 * (mid - left), value);
            destroy_end = right;
            alloc_traits.construct(allocator, data + curr, op(std::move(left_val), std::move(right_val)));
        } catch (..) {
            destroy(left, destroy_end, allocator);
            throw;
        }
        return data[curr];
    }

    T build(std::size_t left, std::size_t right, std::size_t curr, pointer original) {
        if (right - left == 1) {
            alloc_traits.construct(allocator, data + curr, original[left]);
            return data[curr];
        }
        std::size_t mid = (right - left) / 2 + left;
        std::size_t destroy_end = left;
        try {
            T left_val = build(left, mid, curr + 1, original);
            destroy_end = mid;
            T right_val = build(mid, right, curr + 2 * (mid - left), original);
            destroy_end = right;
            alloc_traits.construct(allocator, data + curr, op(std::move(left_val), std::move(right_val)));
        } catch (..) {
            destroy(left, destroy_end, allocator);
            throw;
        }
        return data[curr];
    }

    template<typename... Args>
    void repeat_initialize_helper(Args&& args...) {
        data = alloc_traits::allocate(allocator, length * 2 - 1);
        try {
            build(0, length, 0, std::forward<Args>(args)...);
        } catch (...) {
            alloc_traits::deallocate(allocator, length * 2 - 1);
            throw;
        }
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
        : length(length), op(op), allocator(allocator) {
        repeat_initialize_helper();
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
        : length(length), op(op), allocator(allocator) {
        repeat_initialize_helper(value);
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
    segment_tree(ForwardIt first, ForwardIt last, const Operator& op = Operator(), const Allocator& allocator = Allocator()) requires std::copy_constructible<T>
        : length(std::distance(first, last)), op(op), allocator(allocator) {
        data = static_cast<T*>(::operator new(sizeof(T) * (length * 2 - 1)));
        pointer temp = static_cast<T*>(::operator new(sizeof(T) * length));
        try {
            std::uninitialized_copy(first, last, temp);
            build(0, length, 0, temp);
        } catch (...) {
            ::operator delete(data);
            ::operator delete(temp);
            throw;
        }
        ::operator delete(temp);
    }

    /**
     * @brief Copy constructor
     * 
     * @param other the segment tree to copy from
     */
    segment_tree(const segment_tree& other) requires std::copy_constructible<T>
        : length(other.length), op(other.op) {
        data = static_cast<T*>(::operator new(sizeof(T) * (length * 2 - 1)));
        try {
            std::uninitialized_copy(other.data, other.data + other.length, data);
        } catch (...) {
            // The raw memory must be deallocated
            ::operator delete(data);
            throw;
        }
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
        delete[] data;
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
    T query_helper(std::size_t first, std::size_t last, std::size_t curr, std::size_t left, std::size_t right) {
        if (first == left && last == right) {
            return data[curr];
        }
        std::size_t mid = (right - left) / 2 + left;
        if (last <= mid) {
            return query_helper(first, last, curr + 1, left, mid);
        }
        if (first >= mid) {
            return query_helper(first, last, curr + 2 * (mid - left), mid, right);
        }
        return op(query_helper(first, mid, curr + 1, left, mid), query_helper(mid, last, curr + 2 * (mid - left), mid, right));
    }

    void update_helper(std::size_t pos, std::size_t curr, std::size_t left, std::size_t right, T&& val) {
        if (left == pos && left + 1 == right) {
            data[curr] = std::move(val);
            return;
        }
        std::size_t mid = (right - left) / 2 + left;
        if (pos < mid) {
            update_helper(pos, curr + 1, left, mid, std::move(val));
        } else {
            update_helper(pos, curr + 2 * (mid - left), mid, right, std::move(val));
        }
        data[curr] = op(data[curr + 1], data[curr + 2 * (mid - left)]);
    }

    void collect_subtrees(vector<vector<std::size_t> >& acc, std::size_t curr, std::size_t left, std::size_t right, std::size_t first, std::size_t last) {
        std::size_t left_index = curr + 1;
        std::size_t mid = (right - left) / 2 + left;
        std::size_t right_index = curr + 2 * (mid - left);
        if (left == first && right == last) {
            acc.push_back({curr, left, right});
            return;
        }
        if (last <= mid) {
            collect_subtrees(acc, left_index, left, mid, first, last);
            return;
        }
        if (mid <= first) {
            collect_subtrees(acc, right_index, mid, right, first, last);
            return;
        }
        collect_subtrees(acc, left_index, left, mid, first, mid);
        collect_subtrees(acc, right_index, mid, right, mid, last);
    }

    template<typename F>
    std::optional<std::size_t> leftmost_helper(std::size_t curr, std::size_t left, std::size_t right, std::optional<T> acc, F& decider) {
        if (right - left == 1) {
            return std::make_optional(left);
        }
        std::size_t left_index = curr + 1;
        std::size_t mid = (right - left) / 2 + left;
        std::size_t right_index = curr + 2 * (mid - left);
        if ((acc && decider(op(*acc, data[left_index]))) || decider(data[left_index])) {
            return leftmost_helper(left_index, left, mid, std::move(acc), decider);
        }
        std::optional<T> new_acc = std::make_optional(acc ? op(*acc, data[left_index]) : data[left_index]);
        return leftmost_helper(right_index, mid, right, std::move(new_acc), decider);
    }

    template<typename F>
    std::optional<std::size_t> rightmost_helper(std::size_t curr, std::size_t left, std::size_t right, std::optional<T> acc, F& decider) {
        if (right - left == 1) {
            return std::make_optional(left);
        }
        std::size_t left_index = curr + 1;
        std::size_t mid = (right - left) / 2 + left;
        std::size_t right_index = curr + 2 * (mid - left);
        if ((acc && decider(op(data[right_index], *acc))) || decider(data[right_index])) {
            return rightmost_helper(right_index, mid, right, std::move(acc), decider);
        }
        std::optional<T> new_acc = std::make_optional(acc ? op(data[right_index], *acc) : data[right_index]);
        return rightmost_helper(left_index, left, mid, std::move(new_acc), decider);
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

    T query(std::size_t first, std::size_t last) {
        if (first == last) {
            throw std::invalid_argument("Cannot query empty range");
        }
        return query_helper(first, last, 0, 0, length);
    }

    template<typename F>
    requires std::regular_invocable<F, T> && std::same_as<bool, std::decay_t<std::invoke_result_t<F, T> > >
    std::optional<std::size_t> leftmost(F decider, std::size_t first, std::size_t last) {
        if (first == last) {
            throw std::invalid_argument("Cannot query empty range");
        }
        vector<vector<std::size_t> > subtrees;
        collect_subtrees(subtrees, 0, 0, length, first, last);
        if (decider(data[subtrees.front()[0]])) {
            return leftmost_helper(subtrees.front()[0], subtrees.front()[1], subtrees.front()[2], std::optional<T>(), decider);
        }
        T acc = data[subtrees.front()[0]];
        for (std::size_t i = 1; i < subtrees.size(); ++i) {
            T new_acc = op(acc, data[subtrees[i][0]]);
            if (decider(new_acc)) {
                return leftmost_helper(subtrees[i][0], subtrees[i][1], subtrees[i][2], std::make_optional(acc), decider);
            }
            acc = std::move(new_acc);
        }
        return std::optional<std::size_t>();
    }

    template<typename F>
    requires std::regular_invocable<F, T> && std::same_as<bool, std::decay_t<std::invoke_result_t<F, T> > >
    std::optional<std::size_t> rightmost(F decider, std::size_t first, std::size_t last) {
        if (first == last) {
            throw std::invalid_argument("Cannot query empty range");
        }
        vector<vector<std::size_t> > subtrees;
        collect_subtrees(subtrees, 0, 0, length, first, last);
        if (decider(data[subtrees.back()[0]])) {
            return rightmost_helper(subtrees.back()[0], subtrees.back()[1], subtrees.back()[2], std::optional<T>(), decider);
        }
        T acc = data[subtrees.back()[0]];
        for (int i = subtrees.size() - 2; i >= 0; --i) {
            T new_acc = op(acc, data[subtrees[i][0]]);
            if (decider(new_acc)) {
                return rightmost_helper(subtrees[i][0], subtrees[i][1], subtrees[i][2], std::make_optional(acc), decider);
            }
            acc = std::move(new_acc);
        }
        return std::optional<std::size_t>();
    }

    void update(std::size_t pos, const_reference val) {
        return update_helper(pos, 0, 0, length, T(val));
    }

    void update(std::size_t pos, T&& val) {
        return update_helper(pos, 0, 0, length, std::move(val));
    }

    void swap(segment_tree& other) noexcept(std::is_nothrow_swappable_v<Operator>) {
        std::swap(length, other.length);
        std::swap(data, other.data);
        std::swap(op, other.op);
        std::swap(allocator, other.allocator);
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
        for (const_pointer it1 = tree1.data + 1, it1 = tree2.data + 1; 
             it1 != tree1.data + length && it2 != tree2.data + length; ++it1, ++it2) {
            if (!(*it1 == *it2)) {
                return false;
            }
        }
        return true;
    }
};
}
