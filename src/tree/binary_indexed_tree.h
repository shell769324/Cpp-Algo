#pragma once

#include <concepts>
#include <utility>
#include <bitset>
#include <iterator>
#include "common.h"

namespace algo {
/**
 * @brief A fixed-size data structure that supports fast range query and update
 * 
 * One can define any associative, invertible binary operator like matrix multiplication,
 * addition, multiplication etc. Range accumulation can be computed in log(n) time.
 * 
 * @tparam T type of the value
 * @tparam Operator an associative binary operator of type T * T -> T
 * @tparam InverseOperator the inverse of the operator.
 *         Denote the operator as f, the inverse operator as f'. They satisfy
 *         f'(<A, true>, f(A, B)) = B
 *         f'(<B, false>, f(A, B)) = A
 *         where A, B are any value of type T
 */
template <typename T, typename Operator, typename InverseOperator>
requires std::regular_invocable<Operator, T, T> && std::regular_invocable<InverseOperator, std::pair<T, bool>, T>
class binary_indexed_tree {
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

private:
    // length of the data array. It is one bigger than the length of the input
    std::size_t length;
    /*
     * If the original input is original (0-indexed), data[i] stores the operation result of 
     * original[j:i] where j is i with the last set bit unset. data[0] is unset
     * For example, if the original has length 6
     *                    0: unset
     *         /        /            \
     *  1:[0:1]      2:[0:2]       4:[0:4]
     *                /          /        \
     *            3:[2:3]     5:[4:5]    6:[4:6]
     *                                      \
     *                                     7:[5:6]
     */
    value_type* data;
    Operator op;
    InverseOperator inverse_op;
    T identity;

    /**
     * @brief Given an array that has raw data, construct a binary indexed tree
     * 
     * It runs in linear time
     * 
     * @param data raw data with one indexed
     * @param limit the length of the array
     * @return T the cumulative sum of the entire array
     */
    T initialize(T* data, int limit) {
        T prev = identity;
        for (int i = 1; i < limit; i *= 2) {
            data[i] = op(prev, data[i]);
            int next_limit = std::min(i, limit - i);
            // prev is op(data[1:i * 2]) = op(data[1:i + 1], data[i + 1:i * 2])
            prev = next_limit == 1 ? data[i] : op(data[i], initialize(data + i, next_limit));
        }
        return prev;
    }

public:
    /**
     * @brief Construct a new binary indexed tree with the identity values
     * 
     * @param size the number of elements in this tree
     * @param op the associative, invertible binary operator
     * @param inverseOp the inverse of the binary operator
     * @param identity identity value with regards to this binary operator
     */
    binary_indexed_tree(std::size_t size, const Operator& op, const InverseOperator& inverseOp, T identity=T())
        requires std::default_initializable<T> 
        : binary_indexed_tree(size, identity, op, inverseOp, identity) {}

    /**
     * @brief Construct a new binary indexed tree that repeats the same specified value
     * 
     * @param size the number of elements in this tree
     * @param value the value to fill the binary indexed tree
     * @param op the associative, invertible binary operator
     * @param inverseOp the inverse of the binary operator
     * @param identity identity value with regards to this binary operator
     */
    binary_indexed_tree(std::size_t size, const_reference value, const Operator& op,
        const InverseOperator& inverseOp, T identity=T()) requires std::copy_constructible<T>
        : length(size + 1), op(op), inverse_op(inverseOp), identity(identity) {
        data = static_cast<T*>(::operator new(sizeof(T) * length));
        try {
            // all filled elements will be destroyed when one constructor call throws an exception
            std::uninitialized_fill_n(data + 1, size, value);
        } catch (...) {
            // The raw memory must be deallocated
            ::operator delete(data);
            throw;
        }
        initialize(data, length);
    }

    /**
     * @brief Copy constructor
     * 
     * @param other the binary indexed tree to copy from
     */
    binary_indexed_tree(const binary_indexed_tree& other) requires std::copy_constructible<T>
        : length(other.length), op(other.op), inverse_op(other.inverse_op), identity(other.identity) {
        data = static_cast<T*>(::operator new(sizeof(T) * length));
        try {
            std::uninitialized_copy(other.data + 1, other.data + other.length - 1, data + 1);
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
     * @param other the binary indexed tree to copy from
     */
    binary_indexed_tree(binary_indexed_tree&& other) noexcept :
        length(1),
        data(nullptr),
        op(other.op),
        inverse_op(other.inverse_op) {
        swap(other);
    }

    /**
     * @brief Construct a new binary indexed tree from a range
     * 
     * The binary indexed tree will have the same content as the range defined
     * by the two iterators
     * 
     * @tparam InputIt the type of the input iterator
     * @param first the inclusive begin of the range
     * @param last the exclusive end of the range
     * @param op the associative, invertible binary operator
     * @param inverseOp the inverse of the binary operator
     * @param identity identity value with regards to this binary operator
     */
    template<std::forward_iterator InputIt>
    binary_indexed_tree(InputIt first, InputIt last, const Operator& op,
        const InverseOperator& inverseOp, T identity=T()) requires std::copy_constructible<T>
        : length(std::distance(first, last) + 1), op(op), inverse_op(inverseOp), identity(identity) {
        data = static_cast<T*>(::operator new(sizeof(T) * length));
        std::uninitialized_copy(first, last, data + 1);
        initialize(data, length);
    }

    /**
     * @brief Destructor
     * 
     * Destroy all data in the binary indexed tree and free memory allocated
     */
    ~binary_indexed_tree() {
        std::unique_ptr<T, deleter<T> > cleaner(data);
        std::destroy(data + 1, data + length);
    }

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another binary indexed tree into this one
     * 
     * Strong exception guarantee
     * 
     * @param other the binary indexed tree to copy from
     * @return a reference to itself
     */
    binary_indexed_tree& operator=(const binary_indexed_tree& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }

        binary_indexed_tree tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Move assignment operator
     * 
     * Move all elements from another binary indexed tree into this one. The given
     * binary indexed tree will be left in a valid yet unspecified state
     * 
     * @param other the binary indexed tree to move from
     * @return a reference to itself
     */
    binary_indexed_tree& operator=(binary_indexed_tree&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief Get the length of this tree
     * 
     * @return std::size_t the number of elements this tree has
     */
    std::size_t size() const noexcept {
        return length - 1;
    }

    /**
     * @brief Compute the cumulative sum of range [begin, end)
     * 
     * Consider the binary representation of begin and end. Ignore the
     * longest common prefix. Let the remaining suffixes be begin' and end'. The
     * time complexity is O(|begin'| + |end'|), where |A| is the number
     * of set bits in A.
     * 
     * @param begin the inclusive begin of the range
     * @param end the exclusive end of the range
     * @return T cumulative sum of the range
     */
    T query(std::size_t begin, std::size_t end) {
        if (begin == end) {
            return identity;
        }
        std::size_t begin_curr = begin;
        std::size_t end_curr = end;
        // If we want [5, 10) for example, we need to get the prefix sum of
        // [0, 4] and [0, 9] and take their difference. These in 1-indexed will
        // [1, 5] and [1, 10].
        T begin_sum = identity;
        T end_sum = identity;
        // The idea is, we think of the begin and end in binary representation.
        // We don't need to compute their complete prefix sums and take the diffference.
        // We can stop as soon as they both arrive at the same node, and take their
        // difference.
        while (begin_curr != end_curr) {
            // Alaways shave off the greater one so we never miss the chance of
            // having them landing on the same node
            if (begin_curr < end_curr) {
                end_sum = op(data[end_curr], end_sum);
                // Shave off the last bit of end
                end_curr -= end_curr & (-end_curr);
            } else {
                begin_sum = op(data[begin_curr], begin_sum);
                // Shave off the last bit of begin
                begin_curr -= begin_curr & (-begin_curr);
            }
        }
        if (begin == 0) {
            return end_sum;
        }
        return inverse_op(std::make_pair(begin_sum, true), end_sum);
    }

    /**
     * @brief Update the value at a given index
     * 
     * It runs at O(logn) where n is the length of the array
     * 
     * @param pos the index
     * @param val the new value
     */
    void update(std::size_t pos, const_reference val) {
        ++pos;
        // The right shift argument for the rightmost set bit in pos
        int last_one = 0;
        // First compute the new value for data[pos], which is op(all nodes...) such that its node index i meets
        // pos = i + (i & (-i)). For example, 24's children will be 23, 22, 20, 16. We can iterate through them
        // by first computing pos - 1 and then iteratively unsetting the last pit
        T old_val = data[pos];
        T acc = val;
        for (std::size_t remain = pos - 1; ((1 << last_one) & pos) == 0; remain -= remain & (-remain), ++last_one) {
            acc = op(data[remain], acc);
        }
        data[pos] = acc;
        std::size_t parent;
        // Update all ancestors of pos. Note that the number of iterations in the for loop is equal to the 
        // number of times we increment last_one and it can go beyond the size of integer. Therefore the
        // while loop is amortized O(logn)
        while ((parent = pos + (pos & (-pos))) < length) {
            // Suppose pos is 22, its parent will be 24. The idea is first compute op(16, 20). We can use this to compute
            // the old op(16, 20, 22_old) and take it out of data[24]. Then we use the same thing to compute op(16, 20, 22_new)
            // use it to compute the new data[24]
            ++last_one;
            T acc = identity;
            for (std::size_t remain = pos - (pos & (-pos)); ((1 << last_one) & parent) == 0; remain -= remain & (-remain), ++last_one) {
                acc = op(data[remain], acc);
            }
            T right = inverse_op(std::make_pair(op(acc, old_val), true), data[parent]);
            old_val = data[parent];
            data[parent] = op(op(acc, data[pos]), right);
            pos = parent;
        }
    }

    void swap(binary_indexed_tree& other) noexcept {
        std::swap(length, other.length);
        std::swap(data, other.data);
        std::swap(op, other.op);
        std::swap(inverse_op, other.inverse_op);
        std::swap(identity, other.identity);
    }
};
}
