#pragma once

#include <new>
#include <memory>
#include <concepts>
#include <iterator>
#include "common.h"
#include "allocator_aware_algorithms.h"

namespace algo {
    
/**
 * @brief A generic unbounded array
 * 
 * @tparam T the type of elements in the array
 */
template<typename T, typename Allocator = std::allocator<T> > 
    requires std::same_as<T, typename std::allocator_traits<Allocator>::value_type>
class vector {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_iterator = const T*;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    using alloc_traits = std::allocator_traits<allocator_type>;
    // The length of the portion of the data array that contain elements
    size_type length;
    // The length of data array
    size_type capacity;
    pointer data;
    allocator_type allocator;

    constexpr static size_type DEFAULT_CAPACITY = 4;
    
    /**
     * @brief Set length, capacity and allocate a raw slab of memory
     *      to the buffer
     * 
     * @param len the length
     * @param cap the capacity
     */
    void constructor_helper(size_type len, size_type cap) {
        length = len;
        capacity = cap;
        data = alloc_traits::allocate(allocator, cap);
    }

    /**
     * @brief resize the data buffer
     * 
     * @param count the new capacity
     */
    void resize_buffer(size_type count) {
        if (capacity == count) {
            return;
        }
        T* old_data = data;
        // Create a clone of the old buffer with move if it is safe
        // Elements beyond capacity won't make into the new data buffer
        data = try_move_construct(data, data + length, count, allocator);
        if (capacity > 0) {
            // old buffer must be deallocated no matter what
            std::unique_ptr<T, deleter<T, allocator_type> > 
                cleaner(old_data, deleter<T, allocator_type>(capacity, allocator));
            destroy(old_data, old_data + length, allocator);
        }
        capacity = count;
    }

    /**
     * @brief Make room for elements to insert after a given position
     * 
     * Move all elements after the position further back
     * 
     * @param pos the position after which space will be created
     * @param distance how far trailing elements need to push back
     */
    void make_room(const_iterator pos, size_type distance) {
        if (distance == 0) {
            return;
        }
        difference_type idx = pos - data;
        // Not optimal but less error prone
        // Could have partially resize the buffer and fill in the data
        // to avoid one copy. It is permissible because resize_buffer
        // is amortized constant
        size_type new_length = length + distance;
        if (new_length > capacity) {
            resize_buffer(new_length * 2);
        }
        difference_type dest_left_bound = idx + distance;
        for (T* to = data + new_length - 1; to >= data + dest_left_bound; to--) {
            T* from = to - distance;
            if (to - data < (long int) length) {
                try_move(to, *from);
            } else {
                try_uninitialized_move(to, *from, allocator);
            }
        }
        // Make sure the yielded space is uninitialized
        difference_type clean_right_bound = std::min<difference_type>(dest_left_bound, length);
        destroy(data + idx, data + clean_right_bound, allocator);
    }


public:
    /**
     * @brief Default constructor
     * 
     * Construct a new empty vector
     */
    vector() : vector(allocator_type()) { }

    /**
     * @brief Construct a new vector object with an allocator
     * 
     * @param allocator the allocator to allocate and deallocate raw memory 
     */
    explicit vector(const allocator_type& allocator) : allocator(allocator) {
        constructor_helper(0, DEFAULT_CAPACITY);
    }

    /**
     * @brief Construct a vector with default constructed elements and an optional allocator
     * 
     * Construct a new vector of given length. All elements are
     * default constructed
     * 
     * @param n the number of default constructed elements
     * @param allocator an optional allocator to allocate and deallocate raw memory 
     */
    explicit vector(size_type n, const allocator_type& allocator = allocator_type()) 
            requires std::default_initializable<T> : allocator(allocator) {
        constructor_helper(n, n);
        try {
            // all filled elements will be destroyed when one constructor throws an exception
            uninitialized_default_construct(data, data + n, this -> allocator);
        } catch (...) {
            // The raw memory must be deallocated
            alloc_traits::deallocate(this -> allocator, data, capacity);
            throw;
        }
    }

    /**
     * @brief Construct a vector with copies a given element and an optional allocator
     * 
     * Construct a new vector of a given length. All elements are
     * copys of the specified element
     * 
     * @param n the length of the vector
     * @param value the element to copy
     * @param allocator an optional allocator to allocate and deallocate raw memory 
     */
    vector(size_type n, const_reference value, const allocator_type& allocator = allocator_type())
            requires std::copy_constructible<T> : allocator(allocator) {
        constructor_helper(n, n);
        try {
            // all filled elements will be destroyed when one constructor throws an exception
            uninitialized_fill(data, data + n, value, this -> allocator);
        } catch (...) {
            // The raw memory must be deallocated
            alloc_traits::deallocate(this -> allocator, data, capacity);
            throw;
        }
    }

    /**
     * @brief Construct a new vector from range with an optional allocator
     * 
     * @tparam InputIt the type of the iterators that specify the range
     * @param first the beginning of the range
     * @param last the end of the range
     * @param allocator an optional allocator to allocate and deallocate raw memory 
     */
    template<std::input_iterator InputIt>
    vector(InputIt first, InputIt last, const allocator_type& allocator = allocator_type()) requires std::copy_constructible<T> 
            : length(0), 
              capacity(0), 
              data(nullptr), 
              allocator(allocator) {
        if (first == last) {
            constructor_helper(0, DEFAULT_CAPACITY);
        } else {
            insert(data, first, last);
        }
    }

    /**
     * @brief Copy constructor
     * 
     * Construct a copy of a given vector without modifying it
     * 
     * @param other the vector to copy from
     */
    vector(const vector& other) requires std::copy_constructible<T> : allocator(other.allocator) {
        constructor_helper(other.length, other.capacity);
        try {
            uninitialized_copy(other.cbegin(), other.cend(), data, allocator);
        } catch (...) {
            // The raw memory must be deallocated
            alloc_traits::deallocate(this -> allocator, data, capacity);
            throw;
        }
    }

    /**
     * @brief Construct a copy of another vector with an allocator
     * 
     * Construct a copy of a given vector without modifying it
     * 
     * @param other the vector to copy from
     * @param allocator the allocator to allocate and deallocate raw memory 
     */
    constexpr vector(const vector& other, const std::type_identity_t<Allocator>& allocator) : allocator(allocator) {
        constructor_helper(other.length, other.capacity);
        try {
            uninitialized_copy(other.cbegin(), other.cend(), data, this -> allocator);
        } catch (...) {
            // The raw memory must be deallocated
            alloc_traits::deallocate(this -> allocator, data, capacity);
            throw;
        }
    }

    /**
     * @brief Move constructor
     * 
     * Construct a copy of a given vector by using its resource. The given
     * vector will be left in a valid yet unspecified state
     * 
     * @param other the vector to move from
     */
    vector(vector&& other) noexcept 
        : length{0},
          capacity{0},
          data{nullptr} {
        swap(other);
    }

    /**
     * @brief Construct a copy of another vector with an allocator by move
     * 
     * Construct a copy of a given vector by using its resource. The given
     * vector will be left in a valid yet unspecified state
     * 
     * @param other the vector to move from
     * @param allocator the allocator to allocate and deallocate raw memory 
     */
    vector(vector&& other, const std::type_identity_t<Allocator>& allocator) noexcept 
        : length{0},
          capacity{0},
          data{nullptr},
          allocator(allocator) {
        std::swap(length, other.length);
        std::swap(capacity, other.capacity);
        std::swap(data, other.data);
    }

    /**
     * @brief Construct a new vector object from an initializer list
     * 
     * Example: vector<int> vec{1, 2, 3, 4, 5}
     * 
     * @param list the initializer list
     */
    vector(std::initializer_list<value_type> list, const allocator_type& allocator = allocator_type()) 
        : vector(list.begin(), list.end(), allocator) {}


    /**
     * @brief Destructor
     * 
     * Destroy all elements in the vector and free memory allocated to
     * the vector
     */
    ~vector() noexcept {
        destroy(data, data + length, allocator);
        alloc_traits::deallocate(allocator, data, capacity);
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
                destroy(data, data + length, allocator);
                uninitialized_copy(other.data, other.data + other.length, data, allocator);
                length = other.length;
                allocator = other.allocator;
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
     * @brief Get a copy of the associated allocator
     */
    allocator_type get_allocator() const noexcept {
        return allocator;
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
        destroy(data, data + length, allocator);
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
        difference_type idx = pos - data;
        make_room(pos, 1);
        // insert value
        alloc_traits::construct(allocator, data + idx, value);
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
        difference_type idx = pos - data;
        make_room(pos, 1);
        // insert value
        alloc_traits::construct(allocator, data + idx, std::move(value));
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
        difference_type idx = pos - data;
        std::ptrdiff_t distance = std::distance(first, last);
        make_room(pos, distance);
        // insert values
        T* to = data + idx;
        uninitialized_copy(first, last, to, allocator);
        length += distance;
        return to;
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
        vector storage(allocator);
        // insert values via push_back
        for (auto& it = first; it != last; ++it) {
            storage.push_back(*it);
        }
        return insert(pos, storage.begin(), storage.end());
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
        try_move(data + idx + 1, data + length, data + idx);
        alloc_traits::destroy(allocator, data + length - 1);
        --length;
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
        difference_type first_idx = first - data;
        difference_type last_idx = last - data;
        if (first == last) {
            return data + last_idx;
        }
        difference_type amount = last_idx - first_idx;
        try_move(data + last_idx, data + length, data + first_idx);
        destroy(data + length - amount, data + length, allocator);
        length -= amount;
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
        alloc_traits::construct(allocator, data + length, value);
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
        alloc_traits::construct(allocator, data + length, std::move(value));
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
        alloc_traits::construct(allocator, data + length, std::forward<Args>(args)...);
        return data[length++];
    }

    /**
     * @brief Remove the last element in the vector and destroy it
     */
    void pop_back() {
        --length;
        alloc_traits::destroy(allocator, data + length);
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
    void resize(size_type count) {
        if (count == length) {
            return;
        }
        if (capacity < count) {
            resize_buffer(count * 2);
        }
        if (count > length) {
            uninitialized_default_construct(data + length, data + count, allocator);
        } else {
            destroy(data + count, data + length, allocator);
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
        std::swap(allocator, other.allocator);
    }

    /**
     * @brief Check equality of two vectors
     * 
     * @param vec1 the first vector
     * @param vec2 the second vector
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const vector& vec1, const vector& vec2) noexcept requires equality_comparable<value_type> {
        return container_equals(vec1, vec2);
    }

    /**
     * @brief Compare two vectors
     * 
     * @param vec1 the first vector
     * @param vec2 the second vector
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const vector& vec1, const vector& vec2) noexcept requires less_comparable<value_type> {
        return container_three_way_comparison(vec1, vec2);
    }

    // For testing purpose
    std::size_t __get_default_capacity() const noexcept {
        return DEFAULT_CAPACITY;
    }

    std::size_t __get_capacity() const noexcept {
        return capacity;
    }
};
}
