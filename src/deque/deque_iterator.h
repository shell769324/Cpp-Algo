#pragma once

#include "src/deque/deque_constants.h"

namespace algo {
template <typename T, bool Reverse=false>
class deque_iterator {

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = std::ptrdiff_t;

private:
    using non_const_T = std::remove_const_t<T>;

    non_const_T** outer_pointer;
    non_const_T* inner_pointer;

public:
    deque_iterator() : deque_iterator(nullptr, nullptr) { }

    deque_iterator(non_const_T** outer_pointer, non_const_T* inner_pointer) :
        outer_pointer(outer_pointer), inner_pointer(inner_pointer) { }

    deque_iterator(const deque_iterator& other) = default;
    deque_iterator(deque_iterator&& other) = default;

private:
    /**
     * @brief Construct a deque iterator from its const counterpart
     * 
     * This constructor is private to disallow users from breaking constness of iterator
     * It is also declared explicit to
     * 1) prevent implicitly calling this dangerous conversion
     * 2) resolve ambiguity of the equality operator overload. Note that if this is not explicit,
     *    the equality operator overloads for the non-const version and the const version will be
     *    equally matching. Making this constructor explicit breaks the tie.
     * 
     * @param other the const iterator to copy from
     */
    explicit deque_iterator(const deque_iterator<const T, Reverse>& other)
        requires (!std::is_const_v<T>) : 
        outer_pointer(other.outer_pointer), inner_pointer(other.inner_pointer) { }

public:
    /**
     * @brief Construct a const deque iterator from non-const counterpart
     * 
     * @param other a non-const deque iterator
     */
    deque_iterator(const deque_iterator<non_const_T, Reverse>& other)
        requires std::is_const_v<T> : 
        outer_pointer(other.outer_pointer), inner_pointer(other.inner_pointer) { }

    /**
     * @brief Convert a reverse iterator to a regular iterator or vice versa
     * 
     * @param other an iterator that has opposite reverseness
     */
    deque_iterator(const deque_iterator<T, !Reverse>& other) :
        outer_pointer(other.outer_pointer), inner_pointer(other.inner_pointer) { }

    deque_iterator& operator=(const deque_iterator& other) = default;
    deque_iterator& operator=(deque_iterator&& other) = default;

    /**
     * @brief Get a reference to current value
     */
    reference operator*() const noexcept {
        return *inner_pointer;
    }

    /**
     * @brief Get a pointer to current value
     */
    pointer operator->() const noexcept {
        return inner_pointer;
    }

private:
    void absolute_increment() noexcept {
        if (inner_pointer - *outer_pointer == CHUNK_SIZE<T> - 1) {
            ++outer_pointer;
            inner_pointer = *outer_pointer;
        } else {
            ++inner_pointer;
        }
    }

    void absolute_decrement() noexcept {
        if (inner_pointer == *outer_pointer) {
            --outer_pointer;
            inner_pointer = *outer_pointer + CHUNK_SIZE<T> - 1;
        } else {
            --inner_pointer;
        }
    }

public:
    /**
     * @brief Increment the iterator
     * 
     * @return a reference to self
     */
    deque_iterator& operator++() noexcept {
        if constexpr (Reverse) {
            absolute_decrement();
        } else {
            absolute_increment();
        }
        return *this;
    }

private:
    void increase(const int& delta) {
        if (delta == 0) {
            return;
        }
        long offset = inner_pointer - *outer_pointer + delta;
        long inner_offset = offset % CHUNK_SIZE<T>;
        outer_pointer += offset / CHUNK_SIZE<T> - (inner_offset < 0);
        if (inner_offset < 0) {
            inner_offset += CHUNK_SIZE<T>;
        }
        inner_pointer = *outer_pointer + inner_offset;
    }

public:
    /**
     * @brief Increment the iterator delta of times
     * 
     * @return a reference to self
     */
    deque_iterator& operator+=(const int& delta) noexcept {
        if constexpr (Reverse) {
            increase(-delta);
        } else {
            increase(delta);
        }
        return *this;
    }
    
    /**
     * @brief Increment the iterator
     * 
     * @return a copy of the iterator before the increment
     */
    deque_iterator operator++(int) noexcept {
        deque_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    /**
     * @brief Increment a copy of the iterator delta of times
     * 
     * @param delta 
     * @return the copy of the iterator after the increments 
     */
    deque_iterator operator+(const int& delta) const noexcept {
        deque_iterator tmp = *this;
        return tmp += delta;
    }

    /**
     * @brief Increment the input iterator delta of times
     * 
     * @param delta 
     * @return the input iterator after the increments 
     */
    friend deque_iterator operator+(const int& delta, deque_iterator it) noexcept {
        return it += delta;
    }

    /**
     * @brief Decrement the iterator
     * 
     * @return a reference to self
     */
    deque_iterator& operator--() noexcept {
        if constexpr (Reverse) {
            absolute_increment();
        } else {
            absolute_decrement();
        }
        return *this;
    }

    /**
     * @brief Decrement the iterator delta number of times
     * 
     * @return a reference to self
     */
    deque_iterator& operator-=(const int& delta) noexcept {
        return *this += -delta;
    }

    /**
     * @brief Decrement the iterator
     * 
     * @return a copy of the iterator before the decrement
     */
    deque_iterator operator--(int) noexcept {
        deque_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    /**
     * @brief Decrement a copy of this iterator delta of times
     * 
     * @param delta 
     * @return the copy of this iterator after the decrements 
     */
    deque_iterator operator-(const int& delta) const noexcept {
        deque_iterator tmp = *this;
        tmp -= delta;
        return tmp;
    }

    /**
     * @brief Compute the distance between this iterator and another iterator
     * 
     * @param other another iterator
     * @return difference_type the number of steps (could be negative) between the
     * this iterator and the other iterator
     */
    difference_type operator-(const deque_iterator& other) const noexcept {
        difference_type res = (outer_pointer - other.outer_pointer) * CHUNK_SIZE<T> + 
            (inner_pointer - *outer_pointer) - (other.inner_pointer - *other.outer_pointer);
        if constexpr (Reverse) {
            return -res;
        }
        return res;
    }

    /**
     * @brief Implement all comparison operators
     */
    std::strong_ordering operator<=>(const deque_iterator& other) const noexcept {
        difference_type offset = *this - other;
        return offset <=> 0;
    }

    /**
     * @brief Swap this iterator and another iterator
     */
    void swap(deque_iterator& other) noexcept {
        std::swap(inner_pointer, other.inner_pointer);
        std::swap(outer_pointer, other.outer_pointer);
    }

    /**
     * @brief Check equality of two iterators
     * 
     * @param it1 the first iterator
     * @param it2 the second iterator
     * @return true if they are equal, false otherwise
     */
    friend bool operator==(const deque_iterator& it1, const deque_iterator& it2) noexcept {
        return it1.outer_pointer == it2.outer_pointer && it1.inner_pointer == it2.inner_pointer;
    }

    template<typename U, typename Allocator>
    requires std::same_as<U, typename std::allocator_traits<Allocator>::value_type>
    friend class deque;

    friend class deque_iterator<T, !Reverse>;
    friend std::conditional<(std::is_const_v<T>), deque_iterator<non_const_T, Reverse>, void>::type;
    friend std::conditional<(!std::is_const_v<T>), deque_iterator<const T, Reverse>, void>::type;
};
}
