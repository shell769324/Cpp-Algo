#pragma once

#include <new>
#include <memory>
#include <concepts>
#include <iostream>
#include <algorithm>
#include <iterator>
#include "common.h"

namespace algo {
    
/**
 * @brief A generic unbounded array
 * 
 * @tparam T the type of elements in the array
 */
template<typename T>
class vector {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_iterator = const T*;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    // The length of the portion of the data array that contain elements
    std::size_t length;
    // The length of data array
    std::size_t capacity;
    value_type* data;

    constexpr static const std::size_t DEFAULT_CAPACITY = 4;
    
    /**
     * @brief Set length, capacity and allocate a raw slab of memory
     *      to the buffer
     * 
     * @param len the length
     * @param cap the capacity
     */
    void constructor_helper(std::size_t len, std::size_t cap) {
        length = len;
        capacity = cap;
        data = static_cast<T*>(::operator new(sizeof(T) * cap));
    }

    /**
     * @brief resize the data buffer
     * 
     * If the new capacity is smaller than the current length,
     * elements beyond capacity will be missing from the new data buffer
     * 
     * @param count the new capacity
     */
    void resize_buffer(std::size_t count) {
        if (capacity == count) {
            return;
        }
        std::size_t new_length = std::min(length, count);
        std::size_t old_length = length;
        T* old_data = data;
        // Create a clone of the old buffer with move if it is safe
        // Elements beyond capacity won't make into the new data buffer
        data = safe_move_construct(data, data + new_length, count);
        capacity = count;
        length = new_length;
        // old buffer must be deallocated no matter what
        std::unique_ptr<T, deleter<T> > cleaner(old_data);
        // Call destructor on all elements (some of which have been moved)
        std::destroy(old_data, old_data + old_length);
    }

    /**
     * @brief Make room for elements to insert after a given position
     * 
     * Move all elements after the position further back
     * 
     * @param pos the position after which space will be created
     * @param distance how far trailing elements need to push back
     */
    void make_room(const_iterator pos, std::size_t distance) {
        if (distance == 0) {
            return;
        }
        size_t idx = pos - data;
        // Not optimal but less error prone
        // Could have partially resize the buffer and fill in the data
        // to avoid one copy. It is permissible because resize_buffer
        // is amortized constant
        std::size_t new_length = length + distance;
        if (new_length > capacity) {
            resize_buffer(new_length * 2);
        }
        std::size_t dest_left_bound = idx + distance;
        for (T* to = data + new_length - 1; to >= data + dest_left_bound; to--) {
            T* from = to - distance;
            if (to - data < (long int) length) {
                safe_move(to, *from);
            } else {
                safe_uninitialized_move(to, *from);
            }
        }
        // Make sure the yielded space is uninitialized
        std::size_t clean_right_bound = std::min<std::size_t>(dest_left_bound, length);
        std::destroy(data + idx, data + clean_right_bound);
    }


public:
    /**
     * @brief Default constructor
     * 
     * Construct a new empty vector
     */
    vector() {
        constructor_helper(0, DEFAULT_CAPACITY);
    }

    /**
     * @brief Construct a vector with default constructed elements
     * 
     * Construct a new vector of given length. All elements are
     * default constructed
     * 
     * @param n the length of the vector
     */
    vector(std::size_t n) requires std::default_initializable<T> {
        constructor_helper(n, n);
        try {
            // all filled elements will be destroyed when one constructor throws an exception
            std::uninitialized_default_construct_n(data, n);
        } catch (...) {
            // The raw memory must be deallocated
            ::operator delete(data);
            throw;
        }
    }

    /**
     * @brief Construct a vector with copied of a given element
     * 
     * Construct a new vector of a given length. All elements are
     * copys of the specified element
     * 
     * @param n the length of the vector
     * @param value the element to copy
     */
    vector(std::size_t n, const_reference value) requires std::copy_constructible<T> {
        constructor_helper(n, n);
        try {
            // all filled elements will be destroyed when one constructor throws an exception
            std::uninitialized_fill_n(data, n, value);
        } catch (...) {
            // The raw memory must be deallocated
            ::operator delete(data);
            throw;
        }
    }

    /**
     * @brief Copy constructor
     * 
     * Construct a copy of a given vector without modifying it
     * 
     * @param other the vector to copy from
     */
    vector(const vector& other) requires std::copy_constructible<T> {
        constructor_helper(other.length, other.capacity);
        try {
            std::uninitialized_copy(other.cbegin(), other.cend(), data);
        } catch (...) {
            // The raw memory must be deallocated
            ::operator delete(data);
            throw;
        }
    }

    /**
     * @brief Move constructor
     * 
     * Construct a copy of a given vector by using its resource. The given
     * vector will be left in a valid yet unspecified state
     * 
     * @param other 
     */
    vector(vector&& other) noexcept :
        length{0},
        capacity{0},
        data{nullptr} {
        swap(other);
    }

    /**
     * @brief Construct a new vector object from range
     * 
     * @tparam InputIt the type of the iterators that specify the range
     * @param first the beginning of the range
     * @param last the end of the range
     */
    template<std::input_iterator InputIt>
    vector(InputIt first, InputIt last) requires std::copy_constructible<T> : vector() {
        insert(data, first, last);
    }

    /**
     * @brief Construct a new vector object from an initializer list
     * 
     * Example: vector<int> vec{1, 2, 3, 4, 5}
     * 
     * @param list the initializer list
     */
    vector(std::initializer_list<value_type> list) : vector(list.begin(), list.end()) {}


    /**
     * @brief Destructor
     * 
     * Destroy all elements in the vector and free memory allocated to
     * the vector
     */
    ~vector() {
        // delete even if destructor throws an exception
        std::unique_ptr<T, deleter<T> > cleaner(data);
        std::destroy(data, data + length);
    }

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another vector into this one
     * 
     * Strong exception guarantee
     * 
     * @param other the vector to copy from
     * @return a reference to itself
     */
    vector& operator=(const vector& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }

        if constexpr (std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_destructible_v<T>) {
            if (capacity >= other.length) {
                std::destroy(data, data + length);
                std::uninitialized_copy(other.data, other.data + other.length, data);
                return *this;
            }
        }
        
        vector tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }


    /**
     * @brief Move assignment operator
     * 
     * Move all elements from another vector into this one. The given
     * vector will be left in a valid yet unspecified state
     * 
     * @param other the vector to move from
     * @return a reference to itself
     */
    vector& operator=(vector&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief get a reference to the nth element in the vector
     * 
     * @param pos the index of the element
     */
    reference operator[](size_t pos) {
        return const_cast<reference>(static_cast<const vector&>(*this)[pos]);
    }

    /**
     * @brief get a constant reference to the nth element in the vector
     * 
     * @param pos the index of the element
     */
    const_reference operator[](size_t pos) const {
        return *(data + pos);
    }

    /**
     * @brief Get a reference to the first element in the vector
     */
    reference front() const {
        return data[0];
    }

    /**
     * @brief Get a reference to the last element in the vector
     */
    reference back() const {
        return data[length - 1];
    }

    /**
     * @brief Get an iterator to the first element
     */
    iterator begin() noexcept {
        return data;
    }

    /**
     * @brief Get a constant iterator to the first element
     */
    const_iterator begin() const noexcept {
        return data;
    }

    /**
     * @brief Get a constant iterator to the first element
     */
    const_iterator cbegin() const noexcept {
        return data;
    }

    /**
     * @brief Get a reverse iterator to the last element
     */
    reverse_iterator rbegin() noexcept {
        return reverse_iterator(data + length);
    }

    /**
     * @brief Get a constant reverse iterator to the last element
     */
    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(data + length);
    }

    /**
     * @brief Get a constant reverse iterator to the last element
     */
    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(data + length);
    }

    /**
     * @brief Get an iterator to one past the last element
     */
    iterator end() noexcept {
        return data + length;
    }

    /**
     * @brief Get a constant iterator to one past the last element
     */
    const_iterator end() const noexcept {
        return data + length;
    }

    /**
     * @brief Get a constant iterator to one past the last element
     */
    const_iterator cend() const noexcept {
        return data + length;
    }

    /**
     * @brief Get a reverse iterator to one prior to the first element
     */
    reverse_iterator rend() noexcept {
        return reverse_iterator(data);
    }

    /**
     * @brief Get a constant reverse iterator to one prior to the first element
     */
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(data);
    }

    /**
     * @brief Get a constant reverse iterator to one prior to the first element
     */
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(data);
    }

    /**
     * @brief Test if this vector is empty
     */
    bool empty() const noexcept {
        return length == 0;
    }

    /**
     * @brief Get the number of elements in the vector
     */
    size_t size() const noexcept {
        return length;
    }

    /**
     * @brief Remove all elements from the vector. The vector will be empty after this operation
     */
    void clear() noexcept {
        std::destroy(data, data + length);
        length = 0;
    }

    /**
     * @brief Create and insert a copy of value before pos
     * 
     * @param pos the iterator the position before which the value will be inserted
     * @param value the value to insert
     * 
     * @return an iterator pointing to the inserted value
     */
    iterator insert(const_iterator pos, const_reference value) requires std::copy_constructible<T> {
        if (pos == end()) {
            push_back(value);
            return end() - 1;
        }
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        make_room(pos, 1);
        // insert value
        new(&data[idx]) T(value);
        ++length;
        return data + idx;
    }

    /**
     * @brief Move a value before pos. The value will be left in an unspecified
     *      state after this operation
     * 
     * @return iterator the position before which the value will be inserted
     * @param value the value to insert
     * 
     * @return an iterator pointing to the inserted value
     */
    iterator insert(const_iterator pos, value_type&& value) requires std::move_constructible<T> {
        if (pos == end()) {
            push_back(std::move(value));
            return end() - 1;
        }
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        make_room(pos, 1);
        // insert value
        new(&data[idx]) T(std::move(value));
        ++length;
        return data + idx;
    }

    /**
     * @brief Insert elements from a range before pos. The range is specified
     *      by a pair of iterators
     * 
     * @tparam InputIt the type of the input iterator, must satisfy the spec
     *      of std::forward_iterator
     * @param pos the iterator
     * @param first the beginning of the range, pointing to the first element to insert
     * @param last one past the last element to insert
     * @return iterator pointing to the first element inserted, pos if the range is empty 
     */
    template<std::forward_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) requires std::copy_constructible<T> {
        if (first == last) {
            return const_cast<T*>(pos);
        }
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        std::size_t distance = std::distance(first, last);
        make_room(pos, distance);
        // insert values
        T* to = data + idx;
        std::uninitialized_copy(first, last, to);
        length += distance;
        return data + idx;
    }

    /**
     * @brief Insert elements from a range before pos. The range is specified
     *      by a pair of iterators
     * 
     * @tparam InputIt the type of the input iterator must satisfy the spec
     *      of std::input_iterator
     * @param pos the iterator
     * @param first the beginning of the range, pointing to the first element to insert
     * @param last one past the last element to insert
     * @return iterator pointing to the first element inserted, pos if the range is empty 
     */
    template<std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) requires std::copy_constructible<T> {
        if (first == last) {
            return const_cast<T*>(pos);
        }
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        std::size_t suffix_length = length - idx;

        T* suffix_copy = safe_move_construct(data + idx, data + length, suffix_length);
        std::unique_ptr<T, safe_move_construct_deleter<T>> cleaner(suffix_copy, 
            safe_move_construct_deleter<T>(suffix_length));

        // effectively erase the elements past the point of insertion
        std::destroy(data + idx, data + length);
        length = idx;

        // insert values via push_back
        for (auto& it = first; it != last; ++it) {
            push_back(*it);
        }

        // insert back those elements that were after the point of insertion
        insert(data + length, suffix_copy, suffix_copy + suffix_length);
        return data + idx;
    }

    /**
     * @brief Erase the element at a position
     * 
     * @param pos the position
     * @return iterator to the next element after the erased one.
     *      If the last element is removed, end() will be returned
     */
    iterator erase(const_iterator pos) {
        int idx = pos - data;
        safe_move(data + idx + 1, data + length, data + idx);
        std::destroy_at(data + length - 1);
        --length;
        if (length < capacity / 4) {
            resize_buffer(capacity / 2);
        }
        return data + idx;
    }

    /**
     * @brief Erase all elements in a range
     * 
     * If range is empty, this operation is a noop
     * 
     * @param first the first element to remove
     * @param last the element past the last element to remove
     * 
     * @return iterator to the next element after the erased one.
     *      If the last element is removed, end() will be returned
     */
    iterator erase(const_iterator first, const_iterator last) {
        std::size_t first_idx = first - data;
        std::size_t last_idx = last - data;
        if (first == last) {
            return data + last_idx;
        }
        std::size_t amount = last_idx - first_idx;
        safe_move(data + last_idx, data + length, data + first_idx);
        std::destroy(data + length - amount, data + length);
        length -= amount;
        if (length < capacity / 4) {
            resize_buffer(capacity / 2);
        }
        return data + first_idx;
    }

    /**
     * @brief Create and append a copy of the given value to the end
     *      of the vector
     * 
     * @param value the value to copy and append
     */
    void push_back(const_reference value) requires std::copy_constructible<T> {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(value);
        ++length;
    }

    /**
     * @brief Move given value to the end of the vector
     * 
     * The value will be left in an unspecified yet valid state
     * 
     * @param value the value to move from
     */
    void push_back(value_type&& value) requires std::move_constructible<T> {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(std::move(value));
        ++length;
    }

    /**
     * @brief Construct a new element from the arguments and append it to the
     *      end of the container
     * 
     * @tparam Args an variadic type for any combination of arguments
     * @param args the argument used to construct the element
     * @return reference to the appended element
     */
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(std::forward<Args>(args)...);
        return data[length++];
    }

    /**
     * @brief Remove the last element in the vector and destroy it
     */
    void pop_back() {
        --length;
        data[length].~T();
        if (length == capacity / 4) {
            resize_buffer(std::max(capacity / 2, DEFAULT_CAPACITY));
        }
    }

    /**
     * @brief Resize the container.
     * 
     * If there are currently more elements than count, only the first count
     * number of elements will be kept. If there are less elements than count,
     * default constructed elements will be appended.
     *
     * The capacity is never reduced 
     * 
     * @param count the new size of the container
     */
    void resize(std::size_t count) {
        if (count == length) {
            return;
        }
        if (capacity < count) {
            resize_buffer(count * 2);
        }
        if (count > length) {
            std::uninitialized_default_construct(data + length, data + count);
        } else {
            std::destroy(data + count, data + length);
        }
        length = count;
    }

    /**
     * @brief Swap the content with another vector
     * 
     * @param other the vector to swap with
     */
    void swap(vector& other) noexcept {
        std::swap(length, other.length);
        std::swap(capacity, other.capacity);
        std::swap(data, other.data);
    }
};
}
