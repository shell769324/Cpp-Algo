#pragma once

namespace algo {
/**
 * @brief Iterator class for vector
 * 
 * We need a class type for vector iterator instead of a vanilla pointer. The reason is for overload resolution.
 * Users should be able to implement different overloads for vector iterator and pointer type
 * 
 * @tparam T the value type of the vector
 * @tparam Reverse true if this is a reverse iterator
 */
template <typename T, bool Reverse=false>
class vector_iterator {

public:
    using iterator_category = std::contiguous_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = std::ptrdiff_t;

private:
    using non_const_T = std::remove_const_t<T>;

    non_const_T* data;
public:
    vector_iterator() : vector_iterator(nullptr) { }
    vector_iterator(non_const_T* data) : data(data) { }

public:
    vector_iterator(const vector_iterator& other) = default;
    vector_iterator(vector_iterator&& other) = default;

private:
    /**
     * @brief Construct a vector iterator from its const counterpart
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
    explicit vector_iterator(const vector_iterator<const T, Reverse>& other)
        requires (!std::is_const_v<T>) : data(other.data) { }

public:
    /**
     * @brief Construct a const vector iterator from non-const counterpart
     * 
     * @param other a non-const vector iterator
     */
    vector_iterator(const vector_iterator<non_const_T, Reverse>& other)
        requires std::is_const_v<T> : data(other.data) { }

    /**
     * @brief Convert a reverse iterator to a regular iterator or vice versa
     * 
     * @param other an iterator that has opposite reverseness
     */
    vector_iterator(const vector_iterator<T, !Reverse>& other) : data(other.data) { }

    vector_iterator& operator=(const vector_iterator& other) = default;
    vector_iterator& operator=(vector_iterator&& other) = default;

    reference operator[](difference_type pos) noexcept {
        if constexpr (Reverse) {
            return data[-pos];
        } else {
            return data[pos];
        }
    }

    /**
     * @brief Get a reference to the value at an offset
     */
    reference operator[](difference_type pos) const noexcept {
        if constexpr (Reverse) {
            return data[-pos];
        } else {
            return data[pos];
        }
    }

    /**
     * @brief Get a reference to current value
     */
    reference operator*() const noexcept {
        return *data;
    }

    /**
     * @brief Get a pointer to current value
     */
    pointer operator->() const noexcept {
        return data;
    }

public:
    /**
     * @brief Increment the iterator
     * 
     * @return a reference to self
     */
    vector_iterator& operator++() noexcept {
        if constexpr (Reverse) {
            --data;
        } else {
            ++data;
        }
        return *this;
    }
public:
    /**
     * @brief Increment the iterator delta of times
     * 
     * @return a reference to self
     */
    vector_iterator& operator+=(const int& delta) noexcept {
        if constexpr (Reverse) {
            data -= delta;
        } else {
            data += delta;
        }
        return *this;
    }
    
    /**
     * @brief Increment the iterator
     * 
     * @return a copy of the iterator before the increment
     */
    vector_iterator operator++(int) noexcept {
        vector_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    /**
     * @brief Increment a copy of the iterator delta of times
     * 
     * @param delta 
     * @return the copy of the iterator after the increments 
     */
    vector_iterator operator+(const int& delta) const noexcept {
        vector_iterator tmp = *this;
        return tmp += delta;
    }

    /**
     * @brief Increment the input iterator delta of times
     * 
     * @param delta 
     * @return the input iterator after the increments 
     */
    friend vector_iterator operator+(const int& delta, vector_iterator it) noexcept {
        return it += delta;
    }

    /**
     * @brief Decrement the iterator
     * 
     * @return a reference to self
     */
    vector_iterator& operator--() noexcept {
        if constexpr (Reverse) {
            ++data;
        } else {
            --data;
        }
        return *this;
    }

    /**
     * @brief Decrement the iterator delta number of times
     * 
     * @return a reference to self
     */
    vector_iterator& operator-=(const int& delta) noexcept {
        if constexpr (Reverse) {
            data += delta;
        } else {
            data -= delta;
        }
        return *this;
    }

    /**
     * @brief Decrement the iterator
     * 
     * @return a copy of the iterator before the decrement
     */
    vector_iterator operator--(int) noexcept {
        vector_iterator tmp = *this;
        --(*this);
        return tmp;
    }

    /**
     * @brief Decrement a copy of this iterator delta of times
     * 
     * @param delta 
     * @return the copy of this iterator after the decrements 
     */
    vector_iterator operator-(const int& delta) const noexcept {
        vector_iterator tmp = *this;
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
    difference_type operator-(const vector_iterator& other) const noexcept {
        if constexpr (Reverse) {
            return other.data - data;
        } else {
            return data - other.data;;
        }
    }

    /**
     * @brief Implement all comparison operators
     */
    std::strong_ordering operator<=>(const vector_iterator& other) const noexcept {
        if constexpr (Reverse) {
            return other.data <=> data;
        } else {
            return data <=> other.data;;
        }
    }

    /**
     * @brief Swap this iterator and another iterator
     */
    void swap(vector_iterator& other) noexcept {
        std::swap(data, other.data);
    }

    /**
     * @brief Check equality of two iterators
     * 
     * @param it1 the first iterator
     * @param it2 the second iterator
     * @return true if they are equal, false otherwise
     */
    friend bool operator==(const vector_iterator& it1, const vector_iterator& it2) noexcept {
        return it1.data == it2.data;
    }

    template<typename U, typename Allocator>
    requires std::same_as<U, typename std::allocator_traits<Allocator>::value_type>
    friend class vector;

    friend class vector_iterator<T, !Reverse>;
    friend std::conditional<(std::is_const_v<T>), vector_iterator<non_const_T, Reverse>, void>::type;
    friend std::conditional<(!std::is_const_v<T>), vector_iterator<const T, Reverse>, void>::type;
};
}
