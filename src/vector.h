#include <new>
#include <memory>
#include <concepts>
#include <iostream>
#include <algorithm>

/**
 * @brief 
 * TODO: add erase, initialize list constructor,
 * 
 */
namespace algo {

template<typename T>
class vector {
public:
    using value_type = T;
    using reference = T&;
    using pointer = T*;
    using iterator = T*;
    using const_iterator = const T*;

private:
    std::size_t length;
    std::size_t capacity;
    value_type* data;

    constexpr static const std::size_t DEFAULT_CAPACITY = 4;

    struct deleter {
        void operator()(T* memory) {
            ::operator delete(memory);
        };
    };
    
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

    void resize_buffer(std::size_t count) {
        if (capacity == count) {
            return;
        }
        std::size_t new_length = std::min(length, count);
        std::size_t old_length = length;
        T* old_data = data;
        data = safe_clone(data, data + new_length, count);
        capacity = count;
        length = new_length;
        // old buffer must be deallocated no matter what
        std::unique_ptr<T, deleter> cleaner(old_data, deleter());
        // Call destructor on all elements (some of which have been moved)
        std::destroy(old_data, old_data + old_length);
    }

    /**
     * @brief Create an array of a given size with the same content as in the
     * range
     * 
     * If the type of the value has a nothrwo move constructor, the content in the
     * range will be moved
     * 
     * @param start the beginning of the range
     * @param end the end of the range
     * @param dest_size the size of the clone
     * @return pointer the clone
     */
    pointer safe_clone(T* start, T* end, std::size_t dest_size) {
        T* dest = static_cast<T*>(::operator new(sizeof(T) * dest_size));
        std::unique_ptr<T, deleter> cleaner(dest, deleter());
        if constexpr (std::is_nothrow_move_constructible<T>::value) {
            std::uninitialized_move(start, end, dest);
        } else {
            // If a nothrow move constructor doesn't exist, fall back to copy constructor
            std::uninitialized_copy(start, end, dest);
        }
        cleaner.release();
        return dest;
    }

    /**
     * @brief Construct a copy of a given object on an address
     * 
     * The given object is moved if its type has a nothrow move constructor
     * 
     * @param pos the address to construct the copy
     * @param value the object to copy
     */
    void safe_construct(T* pos, T& value) {
        if constexpr (std::is_nothrow_move_constructible<T>::value) {
            new(pos) T(std::move(value));
        } else {
            new(pos) T(value);
        }
    }

    /**
     * @brief Replace the content of an object at a given address with another
     * object
     * 
     * The given object is moved if its type has a nothrow move constructor
     * 
     * @param pos the address of the object to assigned to
     * @param value the object to assigned from
     */
    void safe_assign(T* pos, T& value) {
        if constexpr (std::is_nothrow_move_assignable<T>::value) {
            *pos = std::move(value);
        } else {
            *pos = value;
        }
    }

    void insert_move_helper(const_iterator pos, std::size_t distance) {
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
        for (T* to = data + new_length - 1; to >= data + idx + distance; to--) {
            T* from = to - distance;
            if (to - data < length) {
                safe_assign(to, *from);
            } else {
                safe_construct(to, *from);
            }
        }
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
    vector(std::size_t n, const T& value) requires std::copy_constructible<T> {
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
    template<class InputIt>
    vector(InputIt first, InputIt last) : vector() {
        insert(data, first, last);
    }

    /**
     * @brief Construct a new vector object from an initializer list
     * 
     * Example: vector<int> vec{1, 2, 3, 4, 5}
     * 
     * @param list the initializer list
     */
    vector(std::initializer_list<T> list) : vector(list.begin(), list.end()) {}


    /**
     * @brief Destructor
     * 
     * Destroy all elements in the vector and free memory allocated to
     * the vector
     */
    ~vector() {
        // delete even if destructor throws an exception
        std::unique_ptr<T, deleter> cleaner(data, deleter());
        std::destroy(data, data + length);
    }

    /**
     * @brief Assignment operator
     * 
     * Copy all elements from another vector into this one without modifying it
     * 
     * @param other the vector to copy from
     * @return a reference to itself
     */
    vector& operator=(const vector& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
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
     * @param n the index of the element
     */
    reference operator[](size_t n) {
        return *(data + n);
    }

    /**
     * @return a reference to the first element in the vector
     */
    reference front() const {
        return data[0];
    }

    /**
     * @return a reference to the last element in the vector
     */
    reference back() const {
        return data[length - 1];
    }

    /**
     * @return a read/write iterator to the first element
     */
    iterator begin() noexcept {
        return data;
    }

    /**
     * @return a read-only iterator to the first element
     */
    const_iterator cbegin() const noexcept {
        return data;
    }

    /**
     * @return an iterator to one past the last element
     */
    iterator end() noexcept {
        return data + length;
    }

    /**
     * @return a read-only iterator to one past the last element
     */
    const_iterator cend() const noexcept {
        return data + length;
    }

    /**
     * @return true if the vector is empty, false otherwise
     */
    bool empty() const noexcept {
        return length == 0;
    }

    /**
     * @return the number of elements in the vector
     */
    size_t size() const noexcept {
        return length;
    }

    /**
     * @brief remove all elements from the vector. The
     * vector will be empty after this operation
     */
    void clear() noexcept {
        std::destroy(data, data + length);
        length = 0;
    }

    /**
     * @brief create and insert a copy of value before pos
     * 
     * @param pos the iterator the position before which the value will be inserted
     * @param value the value to insert
     * 
     * @return an iterator pointing to the inserted value
     */
    iterator insert(const_iterator pos, const T& value) {
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        insert_move_helper(pos, 1);
        // insert value
        data[idx] = value;
        length++;
        return data + idx;
    }

    /**
     * @brief move a value before pos. The value will be left in an unspecified
     * state after this operation
     * 
     * @return iterator the position before which the value will be inserted
     * @param value the value to insert
     * 
     * @return an iterator pointing to the inserted value
     */
    iterator insert(const_iterator pos, T&& value) {
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        insert_move_helper(pos, 1);
        // insert value
        data[idx] = std::move(value);
        length++;
        return data + idx;
    }

    /**
     * @brief insert elements from a range before pos. The range is specified
     * by a pair of iterators
     * 
     * @tparam InputIt the type of the input iterator
     * @param pos the iterator
     * @param first the beginning of the range, pointing to the first element to insert
     * @param last one past the last element to insert
     * @return iterator pointing to the first element inserted, pos if the range is empty 
     */
    template<typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        if (first == last) {
            return &data[pos - data];
        }
        // get the idx before the iterator is invalidated
        std::size_t idx = pos - data;
        std::size_t distance = std::distance(first, last);
        insert_move_helper(pos, distance);
        // insert values
        T* to = data + idx;
        for (InputIt from = first; from != last; from++, to++) {
            if (to - data < length) {
                *to = *from;
            } else {
                new(to) T(*from);
            }
        }
        length += distance;
        return data + idx;
    }

    /**
     * @brief erase an element at a position
     * 
     * @param pos the position
     * @return iterator to the next element after the erased one.
     *      If the last element is removed, end() will be returned
     */
    iterator erase(const_iterator pos) {
        int idx = pos - data;
        if constexpr (std::is_nothrow_move_assignable<T>::value) {
            std::move(data + idx + 1, data + length, data + idx);
        } else {
            // If a nothrow move assignment operator doesn't exist,
            // fall back to regular assignment operator
            std::copy(data + idx + 1, data + length, data + idx);
        }
        std::destroy_at(data + length);
        length--;
        if (length < capacity / 4) {
            resize_buffer(capacity / 2);
        }
        return data + idx;
    }

    /**
     * @brief erase all elements in range [first, last)
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
        int first_idx = first - data;
        int last_idx = last - data;
        if (first == last) {
            return data + last_idx;
        }
        if constexpr (std::is_nothrow_move_assignable<T>::value) {
            std::move(data + last_idx, data + length, data + first_idx);
        } else {
            // If a nothrow move assignment operator doesn't exist,
            // fall back to regular assignment operator
            std::copy(data + last_idx, data + length, data + first_idx);
        }
        std::destroy(data + first_idx, data + last_idx);
        length -= (last_idx - first_idx);
        if (length < capacity / 4) {
            resize_buffer(capacity / 2);
        }
        return data + first_idx;
    }

    /**
     * @brief create and append a copy of the given value to the end
     * of the vector
     * 
     * @param value the value to copy and append
     */
    void push_back(const T& value) requires std::copy_constructible<T> {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(value);
        length++;
    }

    /**
     * @brief move given value to the end of the vector
     * 
     * The value will be left in an unspecified yet valid state
     * 
     * @param value the value to move from
     */
    void push_back(T&& value) requires std::move_constructible<T> {
        // If we are at capacity
        if (length == capacity) {
            resize_buffer(capacity * 2);
        }
        new(data + length) T(std::move(value));
        length++;
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
        length--;
        data[length].~T();
        if (length == capacity / 4) {
            resize_buffer(std::max(capacity / 2, DEFAULT_CAPACITY));
        }
    }

    /**
     * @brief resize the container.
     * 
     * If there are currently more elements than count, only the first count
     * number of elements will be kept. If there are less elements than count,
     * default constructed elements will be appended.
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
            if (count < capacity / 4) {
                resize_buffer(capacity / 2);
            }
        }
        length = count;
    }

    /**
     * @brief swap the content with another vector
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
