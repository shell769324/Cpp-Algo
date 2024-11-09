#pragma once

#include "common.h"

namespace algo {

/**
 * @brief A generic stack
 * 
 * @tparam T the type of elements in the stack 
 */
template <typename T>
class stack {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = T const&;
    using pointer = T*;

private:
    // The length of the portion of the data array that contain elements
    std::size_t length;
    // The length of data array
    std::size_t capacity;
    value_type* data;

    constexpr static const std::size_t DEFAULT_CAPACITY = 4;
    
    /**
     * @brief Set length, capacity and allocate a raw slab of memory
     * to the buffer
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
        std::unique_ptr<T, deleter<T>> cleaner(old_data);
        // Call destructor on all elements (some of which have been moved)
        std::destroy(old_data, old_data + old_length);
    }


public:
    /**
     * @brief Default constructor
     * 
     * Construct a new empty stack
     */
    stack() {
        constructor_helper(0, DEFAULT_CAPACITY);
    }

    /**
     * @brief Construct a stack with default constructed elements
     * 
     * Construct a new stack of given size. All elements are
     * default constructed
     * 
     * @param n the size of the stack
     */
    stack(std::size_t n) requires std::default_initializable<T> {
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
     * @brief Construct a stack with copied of a given element
     * 
     * Construct a new stack of a given length. All elements are
     * copys of the specified element
     * 
     * @param n the length of the stack
     * @param value the element to copy
     */
    stack(std::size_t n, const_reference value) requires std::copy_constructible<T> {
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
     * Construct a copy of a given stack without modifying it
     * 
     * @param other the stack to copy from
     */
    stack(const stack& other) requires std::copy_constructible<T> {
        constructor_helper(other.length, other.capacity);
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
     * Construct a copy of a given stack by using its resource. The given
     * stack will be left in a valid yet unspecified state
     * 
     * @param other 
     */
    stack(stack&& other) noexcept :
        length{0},
        capacity{0},
        data{nullptr} {
        swap(other);
    }

    /**
     * @brief Construct a new stack object from range
     * 
     * @tparam InputIt the type of the iterators that specify the range
     * @param first the beginning of the range
     * @param last the end of the range
     */
    template<class InputIt>
    stack(InputIt first, InputIt last) : stack() {
        for (auto it = first; it != last; ++it) {
            push(*it);
        }
    }

    /**
     * @brief Construct a new stack object from an initializer list
     * 
     * Example: stack<int> vec{1, 2, 3, 4, 5}
     * 
     * @param list the initializer list
     */
    stack(std::initializer_list<value_type> list) : stack(list.begin(), list.end()) {}

    /**
     * @brief Destructor
     * 
     * Destroy all elements in the stack and free memory allocated to
     * the stack
     */
    ~stack() {
        // delete even if destructor throws an exception
        std::unique_ptr<T, deleter<T> > cleaner(data);
        std::destroy(data, data + length);
    }

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another stack into this one without modifying it
     * 
     * @param other the stack to copy from
     * @return a reference to itself
     */
    stack& operator=(const stack& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }
        stack tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }


    /**
     * @brief Move assignment operator
     * 
     * Move all elements from another stack into this one. The given
     * stack will be left in a valid yet unspecified state
     * 
     * @param other the stack to move from
     * @return a reference to itself
     */
    stack& operator=(stack&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @return a reference to the last element in the stack
     */
    reference top() const {
        return data[length - 1];
    }

    /**
     * @return true if the stack is empty, false otherwise
     */
    bool empty() const noexcept {
        return length == 0;
    }

    /**
     * @return the number of elements in the stack
     */
    size_t size() const noexcept {
        return length;
    }

    /**
     * @brief create and push a copy of the given value to the top
     * of the stack
     * 
     * @param value the value to copy and append
     */
    void push(const_reference value) requires std::copy_constructible<T> {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(value);
        ++length;
    }

    /**
     * @brief move and add given value to the top of the stack
     * 
     * The value will be left in an unspecified yet valid state
     * 
     * @param value the value to move from
     */
    void push(value_type&& value) requires std::move_constructible<T> {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(std::move(value));
        ++length;
    }

    /**
     * @brief construct a new element from the arguments and append it to the
     * end of the container
     * 
     * @tparam Args an variadic type for any combination of arguments
     * @param args the argument used to construct the element
     * @return reference to the appended element
     */
    template<typename... Args>
    reference emplace(Args&&... args) {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(std::forward<Args>(args)...);
        return data[length++];
    }

    /**
     * @brief Remove the last element in the stack and destroy it
     */
    void pop() {
        --length;
        data[length].~T();
        if (length == capacity / 4) {
            resize_buffer(std::max(capacity / 2, DEFAULT_CAPACITY));
        }
    }

    /**
     * @brief swap the content with another stack
     * 
     * @param other the stack to swap with
     */
    void swap(stack& other) noexcept {
        std::swap(length, other.length);
        std::swap(capacity, other.capacity);
        std::swap(data, other.data);
    }
};
}
