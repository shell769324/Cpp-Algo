#pragma once

#include "src/deque/deque_constants.h"

namespace algo {
    
template<typename T>
class deque_accessor {
    public:
    std::size_t index;
    const std::size_t& num_chunk;

    deque_accessor(const std::size_t& num_chunk) : index(0), num_chunk(num_chunk) { }

    deque_accessor(const std::size_t& num_chunk, std::size_t index) : 
        index(index), num_chunk(num_chunk) { }

    deque_accessor(const deque_accessor& other) : index(other.index), num_chunk(other.num_chunk) { }

    deque_accessor(deque_accessor&& other) noexcept : index(other.index), num_chunk(other.num_chunk) { }

    deque_accessor& operator=(const deque_accessor& other) {
        index = other.index;
        return *this;
    }

    deque_accessor& operator=(deque_accessor&& other) {
        index = other.index;
        return *this;
    }

    std::strong_ordering operator<=>(const deque_accessor& other) const noexcept {
        return index <=> other.index;
    }

    void operator++(int) noexcept {
        index++;
        if (index == num_chunk * CHUNK_SIZE<T>) {
            index = 0;
        }
    }

    void operator--(int) noexcept {
        if (index == 0) {
            index = num_chunk * CHUNK_SIZE<T> - 1;
            return;
        }
        index--;
    }

    deque_accessor operator+(int delta) const noexcept {
        deque_accessor res = *this;
        res += delta;
        return res;
    }

    deque_accessor& operator+=(int delta) noexcept {
        if (num_chunk == 0) {
            return *this;
        }
        std::size_t capacity = num_chunk * CHUNK_SIZE<T>;
        index += capacity + delta;
        index %= capacity;
        return *this;
    }

    deque_accessor& operator-=(int delta) noexcept {
        return *this += -delta;
    }

    std::size_t inner_index() {
        return index % CHUNK_SIZE<T>;
    }

    std::size_t outer_index() {
        return index / CHUNK_SIZE<T>;
    }

    void swap(deque_accessor& other) {
        std::swap(index, other.index);
    }
};
}
