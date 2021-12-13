#pragma once

namespace algo {

template<typename I, typename T>
concept nothrow_forward_output_iterator =
    std::forward_iterator<I> && std::output_iterator<I, T> && requires(I& it) {
        { *it } noexcept -> std::convertible_to<T&>;
        { it++ } noexcept;
        { ++it } noexcept;
    } && requires(I it1, I it2) {
        { it1 < it2 } noexcept;
        { it1 > it2 } noexcept;
        { it1 <= it2 } noexcept;
        { it1 >= it2 } noexcept;
        { it1 == it2 } noexcept;
    };

template<typename T>
struct deleter {
    void operator()(T* memory) {
        ::operator delete(memory);
    };
};

/**
 * @brief Construct a copy of a given object on an address
 * 
 * The given object is moved if its type has a nothrow move constructor
 * 
 * @param pos the address to construct the copy
 * @param value the object to copy
 */
template<typename T>
void uninitialized_move_safe(T* pos, T& value) {
    if constexpr (std::is_nothrow_move_constructible_v<T>) {
        new(pos) T(std::move(value));
    } else {
        new(pos) T(value);
    }
}

template<typename T>
void uninitialized_move_safe(T* pos, T&& value) {
    // Call the lvalue version
    uninitialized_move_safe(pos, value);
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
template<typename T>
void move_safe(T* pos, T& value) {
    if constexpr (std::is_nothrow_move_assignable_v<T>) {
        *pos = std::move(value);
    } else {
        *pos = value;
    }
}

template<typename T>
void move_safe(T* pos, T&& value) {
    // Call the lvalue version
    move_safe(pos, value);
}


template<std::input_iterator InputIt, typename OutputIt>
void uninitialized_move_safe(InputIt first, InputIt last, OutputIt dest)
    requires nothrow_forward_output_iterator<OutputIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
        std::uninitialized_move(first, last, dest);
    } else {
        // If a nothrow move constructor doesn't exist, fall back to copy constructor
        std::uninitialized_copy(first, last, dest);
    }
}

template<std::input_iterator InputIt, typename OutputIt>
void move_safe(InputIt first, InputIt last, OutputIt dest) 
    requires nothrow_forward_output_iterator<OutputIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
        std::move(first, last, dest);
    } else {
        // If a nothrow move constructor doesn't exist, fall back to copy constructor
        std::copy(first, last, dest);
    }
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
template<std::input_iterator InputIt>
std::iter_value_t<InputIt>* move_construct_safe(InputIt start, InputIt end, std::size_t dest_size) {
    using T = std::iter_value_t<InputIt>;
    T* dest = static_cast<T*>(::operator new(sizeof(T) * dest_size));
    std::unique_ptr<T, deleter<T> > cleaner(dest);
    uninitialized_move_safe(start, end, dest);
    cleaner.release();
    return dest;
}
}