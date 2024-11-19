#pragma once
#include "allocator_aware_algorithms.h"

namespace algo {

/**
 * @brief nothrow_forward_output_iterator as required by a few std methods like
 *      uninitialized_move, move, uninitialized_copy, copy etc
 * 
 * @tparam I the type of the iterator
 * @tparam T the type of the underlying data
 */
template<typename I, typename T>
concept nothrow_forward_output_iterator =
    std::forward_iterator<I> && (std::indirectly_writable<I, T&> || std::indirectly_writable<I, T&&>) && requires(I& it) {
        { *it } noexcept -> std::convertible_to<T&>;
        { it++ } noexcept;
        { ++it } noexcept;
    } && requires(I it1, I it2) {
        { it1 == it2 } noexcept;
    };

template<typename T>
concept equality_comparable =
    requires(const std::remove_reference_t<T>& t) {
        { t == t } -> std::convertible_to<bool>;
    };

template<typename T>
concept less_comparable =
    requires(const std::remove_reference_t<T>& t) {
        { t < t } -> std::convertible_to<bool>;
    };

/**
 * @brief Check if a parameter pack is only a singleton and this type can decay
 *        a specified type or its derived type
 * 
 * @tparam T the type it can decay to
 * @tparam Args the type parameter pack
 */
template<typename T, typename... Args>
concept singleton_pack_decayable_to =
    sizeof...(Args) == 1 &&
        std::derived_from<std::decay_t<std::tuple_element_t<0, std::tuple<Args...> > >, T>;


/**
 * @brief memory deallocator
 * 
 * free the memory (without calling destructor)
 * 
 * @tparam T the type of the underlying data
 * @tparam Allocator the type of the allocator to deallocate memory
 */
template<typename T, typename Allocator>
class deleter {
public:
    using alloc_traits = std::allocator_traits<Allocator>;
    using size_type = Allocator::size_type;

private:
    size_type length;
    Allocator& allocator;

public:
    deleter(size_type length, Allocator& allocator) noexcept : 
        length(length), allocator(allocator) { }

    deleter(const deleter& other) noexcept : deleter(other.length, other.allocator) { }

    deleter(deleter&& other) noexcept : deleter(other.length, other.allocator) { }

    void operator()(T* memory) {
        alloc_traits::deallocate(allocator, memory, length);
    }
};

/**
 * @brief object destroyer and memory deallocator
 * 
 * destroy objects and free the memory
 * 
 * @tparam T the type of the underlying data
 * @tparam Allocator the type of the allocator to deallocate memory
 */
template<typename T, typename Allocator>
class safe_move_construct_deleter {
public:
    using alloc_traits = std::allocator_traits<Allocator>;
    using size_type = alloc_traits::size_type;

private:
    size_type length;
    Allocator& allocator;

public:
    safe_move_construct_deleter(size_type length, Allocator& allocator) : 
        length(length), allocator(allocator) { }

    safe_move_construct_deleter(const safe_move_construct_deleter& other) noexcept : 
        safe_move_construct_deleter(other.length, other.allocator) { }

    safe_move_construct_deleter(safe_move_construct_deleter&& other) noexcept : 
        safe_move_construct_deleter(other.length, other.allocator) { }

    void operator()(T* memory) {
        std::destroy(memory, memory + length);
        alloc_traits::deallocate(allocator, memory, length);
    };
};

/**
 * @brief Construct a copy of a given object on an address
 * 
 * The given object is moved if its type has a move constructor
 * 
 * @param pos the address to construct the copy
 * @param value the object to copy
 * @param allocator the allocator to construct new elements
 */
template<typename T, typename Allocator>
void try_uninitialized_move(T* pos, T& value, Allocator& allocator) {
    if constexpr (std::is_move_constructible_v<T>) {
        std::allocator_traits<Allocator>::construct(allocator, pos, std::move(value));
    } else {
        std::allocator_traits<Allocator>::construct(allocator, pos, value);
    }
}

/**
 * @brief a variant of uninitialized_move_safe that allows the value
 *      to be an rvalue type
 * 
 * @param pos the address to construct the copy
 * @param value the object to copy
 * @param allocator the allocator to construct new elements
 */
template<typename T, typename Allocator>
void try_uninitialized_move(T* pos, T&& value, Allocator& allocator) {
    // Call the lvalue version
    try_uninitialized_move(pos, value, allocator);
}

/**
 * @brief move the elements from the range to an uninitialized memory area
 * 
 * If the type of the underyling data has a move constructor, the elements in the range are moved.
 * Otherwise, they are copied
 * 
 * @tparam InputIt the type of the input iterator, needs to satisfy the std::input_iterator concept
 * @tparam NoThrowForwardIt the output of the input iterator, needs to satisfy the nothrow_forward_output_iterator concept
 * @param first an iterator pointing at the beginning of the range to move from
 * @param last an iterator pointing at one past the last element in the range to move from
 * @param dest the output iterator to move to
 * @param allocator the allocator to construct new elements
 */
template<std::input_iterator InputIt, std::forward_iterator NoThrowForwardIt, typename Allocator>
NoThrowForwardIt try_uninitialized_move(InputIt first, InputIt last, NoThrowForwardIt dest, Allocator& allocator)
    requires nothrow_forward_output_iterator<NoThrowForwardIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    if constexpr (std::is_move_constructible_v<value_type>) {
        return uninitialized_move(first, last, dest, allocator);
    } else {
        // If a nothrow move constructor doesn't exist, fall back to copy constructor
        return uninitialized_copy(first, last, dest, allocator);
    }
}


/**
 * @brief Replace the content of an object at a given address with another object
 * 
 * The given object is moved if its type has a move constructor
 * 
 * @param pos the address of the object to assigned to
 * @param value the object to assigned from
 */
template<typename T>
void try_move(T* pos, T& value) {
    if constexpr (std::is_move_assignable_v<T>) {
        *pos = std::move(value);
    } else {
        *pos = value;
    }
}

/**
 * @brief a variant of move_safe that allows the value
 *      to be an rvalue type
 */
template<typename T>
void try_move(T* pos, T&& value) {
    // Call the lvalue version
    try_move(pos, value);
}


/**
 * @brief move the elements from the range to an initialized memory area
 * 
 * If the type of the underyling data has a move assignment operator, the elements in the range are moved.
 * Otherwise, they are copy assigned
 * 
 * @tparam InputIt the type of the input iterator, needs to satisfy the std::input_iterator concept
 * @tparam OutputIt the output of the input iterator, needs to satisfy the nothrow_forward_output_iterator concept
 * @param first an iterator pointing at the beginning of the range to move from
 * @param last an iterator pointing at one past the last element in the range to move from
 * @param dest the output iterator to move to
 */
template<std::input_iterator InputIt, typename OutputIt>
OutputIt try_move(InputIt first, InputIt last, OutputIt dest) 
    requires nothrow_forward_output_iterator<OutputIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    if constexpr (std::is_move_assignable_v<value_type>) {
        return std::move(first, last, dest);
    } else {
        // If a nothrow move constructor doesn't exist, fall back to copy constructor
        return std::copy(first, last, dest);
    }
}

/**
 * @brief Create an array of a given size with the same elements as in the
 * range
 * 
 * If the type of the value has a move constructor, the elements in the
 * range will be moved. Otherwise, they are copied.
 * 
 * @param start the beginning of the range
 * @param end the end of the range
 * @param dest_size the size of the clone
 * @return pointer the clone
 */
template<std::input_iterator InputIt, typename Allocator>
std::iter_value_t<InputIt>* try_move_construct(InputIt start, InputIt end, 
                                                std::size_t dest_size, Allocator& allocator) {
    using T = std::iter_value_t<InputIt>;
    T* dest = std::allocator_traits<Allocator>::allocate(allocator, dest_size);
    std::unique_ptr<T, deleter<T, Allocator> > cleaner(dest, deleter<T, Allocator>(dest_size, allocator));
    try_uninitialized_move(start, end, dest, allocator);
    cleaner.release();
    return dest;
}

/**
 * @brief Accessor of the left element of a std::pair
 * 
 * @tparam T the type of the left element
 * @tparam U the type of the right element
 */
template <typename T, typename U>
class pair_left_accessor {
public:
    /**
     * @brief Get a const reference to the left element of a pair
     */
    const T& operator()(const std::pair<const T, U>& pair) const {
        return pair.first;
    }
};

template<typename T>
class chooser {
private:
    bool choose_first;
public:
    chooser(bool choose_first=true) : choose_first(choose_first) { }

    /**
     * @brief Always pick the first or the second value
     */
    bool operator()(__attribute__((unused)) const T& value1, __attribute__((unused)) const T& value2) const {
        return choose_first;
    }
};

template<typename Container>
bool container_equals(const Container& container1, const Container& container2) noexcept requires equality_comparable<typename Container::value_type> {
    if (container1.size() != container2.size()) {
        return false;
    }
    using const_iterator = typename Container::const_iterator;
    for (const_iterator it1 = container1.cbegin(), it2 = container2.cbegin(); it1 != container1.cend() && it2 != container2.cend(); ++it1, ++it2) {
        if (!(*it1 == *it2)) {
            return false;
        }
    }
    return true;
}

template<typename Container>
std::strong_ordering container_three_way_comparison(const Container& container1, const Container& container2) noexcept requires less_comparable<typename Container::value_type> {
    using const_iterator = typename Container::const_iterator;
    for (const_iterator it1 = container1.cbegin(), it2 = container2.cbegin(); it1 != container1.cend() && it2 != container2.cend(); ++it1, ++it2) {
        if (*it1 < *it2) {
            return std::strong_ordering::less;
        }
        if (*it1 > *it2) {
            return std::strong_ordering::greater;
        }
    }
    return container1.size() <=> container2.size();
}
}
