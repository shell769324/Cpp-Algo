#pragma once

#include "src/deque/deque_constants.h"
#include "src/deque/deque_accessor.h"

namespace algo {
template <typename T>
class deque_iterator {

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = long signed;

private:
    using non_const_T = std::remove_const_t<T> ;
    using accessor = deque_accessor<non_const_T>;

    non_const_T**& data;
    const accessor& front_index;
    std::size_t offset;

public:
    deque_iterator(non_const_T**& data, const accessor& front_index, std::size_t offset) :
        data(data), front_index(front_index), offset(offset) { }

    deque_iterator(const deque_iterator& other) : data(other.data), front_index(other.front_index), offset(other.offset) { }

    deque_iterator(deque_iterator&& other) : data(other.data), front_index(other.front_index), offset(0) {
        swap(other);
    }

    deque_iterator& operator=(const deque_iterator& other) noexcept {
        offset = other.offset;
        return *this;
    }

    deque_iterator& operator=(deque_iterator&& other) noexcept {
        offset = other.offset;
        return *this;
    }

    /**
     * @brief Get a reference to current value
     */
    reference operator*() const noexcept {
        accessor front_index_copy(front_index);
        front_index_copy += offset;
        return data[front_index_copy.outer_index()][front_index_copy.inner_index()];
    }

    /**
     * @brief Get a pointer to current value
     */
    pointer operator->() const noexcept {
        accessor front_index_copy(front_index);
        front_index_copy += offset;
        return data[front_index_copy.outer_index()] + front_index_copy.inner_index();
    }

    deque_iterator& operator++() noexcept {
        offset++;
        return *this;
    }

    deque_iterator& operator+=(const int& delta) noexcept {
        offset += delta;
        return *this;
    }

    deque_iterator operator++(int) noexcept {
        deque_iterator tmp = *this;
        offset++;
        return tmp;
    }

    deque_iterator operator+(const int& delta) const noexcept {
        deque_iterator tmp = *this;
        tmp += delta;
        return tmp;
    }

    friend deque_iterator operator+(const int& delta, deque_iterator it) noexcept {
        deque_iterator tmp = it;
        tmp += delta;
        return tmp;
    }

    deque_iterator& operator--() noexcept {
        offset--;
        return *this;
    }

    deque_iterator& operator-=(const int& delta) noexcept {
        offset -= delta;
        return *this;
    }

    deque_iterator operator--(int) noexcept {
        deque_iterator tmp = *this;
        offset--;
        return *this;
    }

    deque_iterator operator-(const int& delta) const noexcept {
        deque_iterator tmp = *this;
        tmp -= delta;
        return tmp;
    }

    difference_type operator-(const deque_iterator& other) const noexcept {
        return offset - other.offset;
    }

    std::strong_ordering operator<=>(const deque_iterator& other) const noexcept {
        return offset <=> other.offset;
    }

    void swap(deque_iterator& other) noexcept {
        std::swap(offset, other.offset);
    }

    /**
     * @brief Check equality of two iterators
     * 
     * @param it1 the first iterator
     * @param it2 the second iterator
     * @return true if they are equal, false otherwise
     */
    friend bool operator==(const deque_iterator& it1, const deque_iterator& it2) {
        return it1.offset == it2.offset && it1.data == it2.data;
    }
};
}
