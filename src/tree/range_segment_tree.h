#pragma once

#include <optional>
#include <stdexcept>
#include <concepts>
#include <algorithm>
#include <climits>
#include <iterator>
#include <memory>
#include "src/common.h"
#include "src/vector.h"


namespace algo {

template <typename T, typename Operator, typename RepeatOperator>
requires std::regular_invocable<Operator, T, T> && std::regular_invocable<RepeatOperator, std::size_t, T> &&
    std::same_as<T, std::decay_t<std::invoke_result_t<Operator, T, T> > > &&
    std::same_as<T, std::decay_t<std::invoke_result_t<RepeatOperator, std::size_t, T> > >
class range_segment_tree {
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    
private:
    std::size_t length;
    pointer data;
    pointer lazy_data;
    bool* lazy_flags;
    Operator op;
    RepeatOperator rop;

    T build(std::size_t left, std::size_t right, std::size_t curr, const_reference value) {
        if (right - left == 1) {
            new(data + curr) T(value);
            return value;
        }
        std::size_t mid = (right - left) / 2 + left;
        T left_val = build(left, mid, curr + 1, value);
        T right_val = build(mid, right, curr + 2 * (mid - left), value);
        new(data + curr) T(op(std::move(left_val), std::move(right_val)));
        return data[curr];
    }

    T build(std::size_t left, std::size_t right, std::size_t curr, pointer original) {
        if (right - left == 1) {
            new(data + curr) T(original[left]);
            return data[curr];
        }
        std::size_t mid = (right - left) / 2 + left;
        T left_val = build(left, mid, curr + 1, original);
        T right_val = build(mid, right, curr + 2 * (mid - left), original);
        new(data + curr) T(op(std::move(left_val), std::move(right_val)));
        return data[curr];
    }
    
public:
    range_segment_tree(std::size_t length, const Operator& op = Operator(), const RepeatOperator& rop = RepeatOperator())
        requires std::default_initializable<T> 
        : range_segment_tree(length, T(), op, rop) {}

    range_segment_tree(std::size_t length, const_reference value, const Operator& op=Operator(), const RepeatOperator& rop = RepeatOperator()) requires std::copy_constructible<T>
        : length(length), op(op), rop(rop) {
        std::size_t capacity = length * 2 - 1;
        data = static_cast<T*>(::operator new(sizeof(T) * (capacity)));
        lazy_data = static_cast<T*>(::operator new(sizeof(T) * (capacity)));
        lazy_flags = new bool[capacity];
        std::fill(lazy_flags, lazy_flags + capacity, false);
        try {
            build(0, length, 0, value);
        } catch (...) {
            ::operator delete(data);
            ::operator delete(lazy_data);
            delete[] lazy_flags;
            throw;
        }
    }

    /**
     * @brief Copy constructor
     * 
     * @param other the segment tree to copy from
     */
    range_segment_tree(const range_segment_tree& other) requires std::copy_constructible<T>
        : length(other.length), op(other.op), rop(other.rop) {
        std::size_t capacity = length * 2 - 1;
        data = static_cast<T*>(::operator new(sizeof(T) * capacity));
        lazy_data = static_cast<T*>(::operator new(sizeof(T) * capacity));
        lazy_flags = new bool[capacity];
        try {
            std::uninitialized_copy(other.data, other.data + other.length, data);
            std::uninitialized_copy(other.lazy_data, other.lazy_data + other.length, lazy_data);
            std::copy(other.lazy_flags, other.lazy_flags + other.length, lazy_flags);
        } catch (...) {
            // The raw memory must be deallocated
            ::operator delete(data);
            ::operator delete(lazy_data);
            delete[] lazy_flags;
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
    range_segment_tree(range_segment_tree&& other) noexcept :
        length(0),
        data(nullptr),
        op(other.op),
        rop(other.rop) {
        swap(other);
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
    range_segment_tree(ForwardIt first, ForwardIt last, const Operator& op=Operator(), const RepeatOperator& rop = RepeatOperator()) requires std::copy_constructible<T>
        : length(std::distance(first, last)), op(op), rop(rop) {
        std::size_t capacity = length * 2 - 1;
        data = static_cast<T*>(::operator new(sizeof(T) * capacity));
        lazy_data = static_cast<T*>(::operator new(sizeof(T) * capacity));
        lazy_flags = new bool[capacity];
        std::fill(lazy_flags, lazy_flags + capacity, false);
        pointer temp = static_cast<T*>(::operator new(sizeof(T) * length));
        try {
            std::uninitialized_copy(first, last, temp);
            build(0, length, 0, temp);
        } catch (...) {
            ::operator delete(data);
            ::operator delete(lazy_data);
            ::operator delete(temp);
            delete[] lazy_flags;
            throw;
        }
        ::operator delete(temp);
    }

    /**
     * @brief Destructor
     * 
     * Destroy all data in the segment tree and free memory allocated
     */
    ~range_segment_tree() {
        std::unique_ptr<bool[]> flags_cleaner(lazy_flags);
        delete[] data;
        for (std::size_t i = 0; i < length; ++i) {
            if (lazy_flags[i]) {
                std::destroy_at(lazy_data + i);
            }
        }
        ::operator delete(lazy_data);
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
     * Move all elements from another segment tree into this one. The given
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
    void push(std::size_t curr, std::size_t left, std::size_t right) {
        if (!lazy_flags[curr]) {
            return;
        }
        std::size_t left_index = curr + 1;
        std::size_t mid = (right - left) / 2 + left;
        std::size_t right_index = curr + 2 * (mid - left);
        data[left_index] = rop(mid - left, lazy_data[curr]);
        data[right_index] = rop(right - mid, lazy_data[curr]);
        new(lazy_data + left_index) T(lazy_data[curr]);
        new(lazy_data + right_index) T(lazy_data[curr]);
        lazy_flags[left_index] = true;
        lazy_flags[right_index] = true;
        std::destroy_at(lazy_data + curr);
        lazy_flags[curr] = false;
    }

    T query_helper(std::size_t first, std::size_t last, std::size_t curr, std::size_t left, std::size_t right) {
        if (first == left && last == right) {
            return data[curr];
        }
        push(curr, left, right);
        std::size_t mid = (right - left) / 2 + left;
        if (last <= mid) {
            return query_helper(first, last, curr + 1, left, mid);
        }
        if (first >= mid) {
            return query_helper(first, last, curr + 2 * (mid - left), mid, right);
        }
        return op(query_helper(first, mid, curr + 1, left, mid), query_helper(mid, last, curr + 2 * (mid - left), mid, right));
    }

    void update_helper(std::size_t first, std::size_t last, T&& val, std::size_t curr, std::size_t left, std::size_t right) {
        if (left == first && right == last) {
            data[curr] = rop(right - left, val);
            lazy_data[curr] = std::move(val);
            lazy_flags[curr] = true;
            return;
        }
        push(curr, left, right);
        std::size_t mid = (right - left) / 2 + left;
        if (last <= mid) {
            update_helper(first, last, std::move(val), curr + 1, left, mid);
        } else if (mid <= first) {
            update_helper(first, last, std::move(val), curr + 2 * (mid - left), mid, right);
        } else {
            update_helper(first, mid, std::move(val), curr + 1, left, mid);
            update_helper(mid, last, std::move(val), curr + 2 * (mid - left), mid, right);
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
        push(curr, left, right);
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
        push(curr, left, right);
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
        push(curr, left, right);
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
        return update_helper(pos, pos + 1, T(val), 0, 0, length);
    }

    void update(std::size_t pos, T&& val) {
        return update_helper(pos, pos + 1, std::move(val), 0, 0, length);
    }

    void update(std::size_t first, std::size_t last, const_reference val) {
        return update_helper(first, last, T(val), 0, 0, length);
    }

    void update(std::size_t first, std::size_t last, T&& val) {
        return update_helper(first, last, std::move(val), 0, 0, length);
    }

    void swap(range_segment_tree& other) noexcept {
        std::swap(length, other.length);
        std::swap(data, other.data);
        std::swap(op, other.op);
    }
};
}
