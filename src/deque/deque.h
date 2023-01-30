#pragma once

#include "src/deque/deque_constants.h"
#include "src/deque/deque_accessor.h"
#include "src/deque/deque_iterator.h"

namespace algo {
    
template<typename T>
class deque {
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = deque_iterator<T>;
    using const_iterator = deque_iterator<const T>;


private:
    using accessor = deque_accessor<T>;

    std::size_t num_chunk;
    std::size_t num_elements;

    T** data;
    // Both are inclusive
    accessor front_index;
    accessor tail_index;

    void resize_buffer(std::size_t new_num_chunk) {
        if (new_num_chunk == num_chunk) {
            return;
        }
        T** new_data = new T*[new_num_chunk];
        for (std::size_t i = 0; i < new_num_chunk; i++) {
            new_data[i] = nullptr;
        }
        // front and tail point to the positions to insert
        // next element
        front_index++;
        tail_index--;
        std::size_t count = (front_index.outer_index() <= tail_index.outer_index() ? 0 : num_chunk) + 
            tail_index.outer_index() - front_index.outer_index();
        for (std::size_t i = front_index.outer_index(), j = 0; j <= count; j++) {
            new_data[j] = data[i];
            i++;
            if (num_chunk == i) {
                i = 0;
            }
        }
        num_chunk = new_num_chunk;
        delete[] data;
        data = new_data;
        front_index -= front_index.outer_index() * CHUNK_SIZE<T> + 1;
        tail_index = front_index + (num_elements + 1);
    }

    bool is_valid() {
        return true;
    }

public:
    deque() : 
        num_chunk(DEFAULT_NUM_CHUNK),
        num_elements(0),
        data(new T*[num_chunk]),
        front_index(num_chunk),
        tail_index(num_chunk) {
        for (std::size_t i = 0; i < num_chunk; i++) {
            data[i] = nullptr;
        }
        tail_index++;
    }

    deque(std::size_t n) requires std::default_initializable<T> : deque(n, T()) {}

    deque(std::size_t n, const_reference value) requires std::copy_constructible<T> :
        num_chunk((n + CHUNK_SIZE<T> - 1)/ CHUNK_SIZE<T>),
        num_elements(n),
        data(new T*[num_chunk]),
        front_index(num_chunk),
        tail_index(num_chunk, 1) {
        std::size_t remain = n;
        for (std::size_t i = 0; i < num_chunk; i++, remain -= CHUNK_SIZE<T>) {
            data[i] = static_cast<T*>(::operator new(sizeof(T) * CHUNK_SIZE<T>));
            std::size_t cap = std::min(CHUNK_SIZE<T>, remain);
            std::uninitialized_fill(data[i], data[i] + cap, value);
        }
        tail_index = tail_index + n;
    }

    deque(const deque& other) requires std::copy_constructible<T> : 
        num_chunk(other.num_chunk),
        num_elements(other.num_elements),
        data(new T*[num_chunk]),
        front_index(num_chunk, other.front_index.index),
        tail_index(num_chunk, other.tail_index.index) {
        if (num_elements == 0) {
            return;
        }
        front_index++;
        tail_index--;
        std::size_t cap = (num_chunk + tail_index.outer_index() - front_index.outer_index()) % num_chunk + 1;
        for (std::size_t i = front_index.outer_index(), j = 0; j < cap; j++) {
            data[i] = static_cast<T*>(::operator new(sizeof(T) * CHUNK_SIZE<T>));
            i++;
            if (i == num_chunk) {
                i = 0;
            }
        }
        std::size_t i = 0;
        for (accessor acc = front_index; i < num_elements; i++, acc++) {
            new(data[acc.outer_index()] + acc.inner_index()) T(other.data[acc.outer_index()][acc.inner_index()]);
        }
        front_index--;
        tail_index++;
    }

    deque(deque&& other) noexcept : 
        num_chunk(0),
        num_elements(0),
        data(nullptr),
        front_index(num_chunk),
        tail_index(num_chunk) {
        swap(other);
    }

    template<typename InputIt>
    deque(InputIt first, InputIt last) requires std::copy_constructible<T> : deque() {
        for (auto it = first; it != last; it++) {
            push_back(*it);
        }
    }

    deque& operator=(const deque& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }

        if constexpr (std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_destructible_v<T>) {
            if (num_elements >= other.num_elements) {
                std::size_t i = 0;
                accessor acc = front_index;
                for (; i < other.num_elements; acc++, i++) {
                    data[acc.outer_index()][acc.inner_index()] = other[i];
                }
                for (; i < num_elements; acc++, i++) {
                    data[acc.outer_index()][acc.inner_index()].~T();
                }
                return *this;
            }
        }
        
        deque tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    deque& operator=(deque&& other) noexcept {
        swap(other);
        return *this;
    }

    ~deque() {
        front_index++;
        std::size_t i = 0;
        for (accessor acc = front_index; i < num_elements; i++, acc++) {
            data[acc.outer_index()][acc.inner_index()].~T();
        }
        tail_index--;
        // It is possible data is just a nullptr. It can happen after
        // move constructor
        if (data != nullptr) {
            std::size_t cap = (num_chunk + tail_index.outer_index() - front_index.outer_index()) % num_chunk + 1;
            for (std::size_t i = front_index.outer_index(), j = 0; j < cap; j++) {
                // Defensive deallocation
                if (data[i] != nullptr) {
                    ::operator delete[](data[i]);
                }
                i++;
                if (i == num_chunk) {
                    i = 0;
                }
            }
            delete[] data;
        }
    }

    reference operator[](std::size_t pos) {
        return const_cast<reference>(static_cast<const deque&>(*this)[pos]);
    }

    const_reference operator[](std::size_t pos) const {
        accessor acc = front_index + pos + 1;
        return data[acc.outer_index()][acc.inner_index()];
    }

    /**
     * @brief Get a reference to the first element in the deque
     */
    reference front() const {
        accessor acc = front_index + 1;
        return data[acc.outer_index()][acc.inner_index()];
    }

    /**
     * @brief Get a reference to the last element in the deque
     */
    reference back() const {
        accessor acc = front_index + num_elements;
        return data[acc.outer_index()][acc.inner_index()];
    }


    iterator begin() {
        return iterator(data, front_index, 1);
    }

    /**
     * @brief Get a constant iterator to the first element
     */
    const_iterator begin() const noexcept {
        return const_iterator(const_cast<T**&>(data), front_index, 1);;
    }

    /**
     * @brief Get a constant iterator to the first element
     */
    const_iterator cbegin() const noexcept {
        return const_iterator(const_cast<T**&>(data), front_index, 1);;
    }

    iterator end() {
        return iterator(data, front_index, 1 + num_elements);
    }

    const_iterator end() const noexcept{
        return const_iterator(const_cast<T**&>(data), front_index, 1 + num_elements);
    }

    const_iterator cend() const noexcept{
        return const_iterator(const_cast<T**&>(data), front_index, 1 + num_elements);
    }

    bool empty() const noexcept {
        return num_elements == 0;
    }

    std::size_t size() const noexcept {
        return num_elements;
    }

private:
    void allocate_tail_space() {
        if (front_index.outer_index() == tail_index.outer_index() && num_elements >= CHUNK_SIZE<T>) {
            resize_buffer(num_chunk * 2);
        }
        if (data[tail_index.outer_index()] == nullptr) {
            data[tail_index.outer_index()] = static_cast<T*>(::operator new(sizeof(T) * CHUNK_SIZE<T>));
        }
    }
    
public:
    void push_back(const_reference value) requires std::copy_constructible<T> {
        allocate_tail_space();
        new(data[tail_index.outer_index()] + tail_index.inner_index()) T(value);
        tail_index++;
        num_elements++;
    }

    template<typename... Args>
    reference emplace_back(Args&&... args) {
        allocate_tail_space();
        new(data[tail_index.outer_index()] + tail_index.inner_index()) T(std::forward<Args>(args)...);
        reference res = data[tail_index.outer_index()][tail_index.inner_index()];
        tail_index++;
        num_elements++;
        return res;
    }

    void pop_back() {
        tail_index--;
        data[tail_index.outer_index()][tail_index.inner_index()].~T();
        num_elements--;
    }

private:
    void allocate_front_space() {
        if (front_index.outer_index() == tail_index.outer_index() && num_elements >= CHUNK_SIZE<T>) {
            resize_buffer(num_chunk * 2);
        }
        if (data[front_index.outer_index()] == nullptr) {
            data[front_index.outer_index()] = static_cast<T*>(::operator new(sizeof(T) * CHUNK_SIZE<T>));
        }
    }
    
public:
    void push_front(const_reference value) requires std::copy_constructible<T> {
        allocate_front_space();
        new(data[front_index.outer_index()] + front_index.inner_index()) T(value);
        front_index--;
        num_elements++;
    }

    template<typename... Args>
    reference emplace_front(Args&&... args) {
        allocate_front_space();
        new(data[front_index.outer_index()] + front_index.inner_index()) T(std::forward<Args>(args)...);
        reference res = data[front_index.outer_index()][front_index.inner_index()];
        front_index--;
        num_elements++;
        return res;
    }

    void pop_front() {
        front_index++;
        data[front_index.outer_index()][front_index.inner_index()].~T();
        num_elements--;
    }

    void swap(deque& other) noexcept {
        std::swap(data, other.data);
        std::swap(num_chunk, other.num_chunk);
        std::swap(front_index, other.front_index);
        std::swap(tail_index, other.tail_index);
        std::swap(num_elements, other.num_elements);
    }
};
}