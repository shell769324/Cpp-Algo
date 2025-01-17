#pragma once
#include "allocator_aware_algorithms.h"
#include "concepts.h"
#include <cstring>
#include <algorithm>

namespace algo {

/**
 * @brief Check if a pair of input and output iterators are eligible for memcpy/memmove
 * 
 * Both must be pointer type with the same value type (the input value type may be additionally const qualified)
 * The object must be trivially copy constructible and trivially destructible.
 * The value type cannot be volatile
 * 
 * @tparam OutputIt the type of the output iterator
 * @tparam InputIt the type of the input iterator
 */
template<typename OutputIt, typename InputIt>
struct is_memcpyable {
    static constexpr bool value = 
        std::is_pointer_v<OutputIt> && std::is_pointer_v<InputIt> 
        && std::is_same_v<std::remove_pointer_t<OutputIt>, std::remove_const_t<std::remove_pointer_t<InputIt>>>
        && !std::is_volatile_v<std::remove_pointer_t<OutputIt> > 
        && std::is_trivially_copy_constructible_v<std::iter_value_t<OutputIt> > 
        && std::is_trivially_destructible_v<std::iter_value_t<OutputIt>>;
};

template<typename OutputIt, typename InputIt>
inline constexpr bool is_memcpyable_v = is_memcpyable<OutputIt, InputIt>::value;

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

    size_type size() const noexcept {
        return length;
    }
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
template<bool NoThrowMove, std::input_iterator InputIt, std::forward_iterator NoThrowForwardIt, typename Allocator>
NoThrowForwardIt try_uninitialized_move(InputIt first, InputIt last, NoThrowForwardIt dest, Allocator& allocator)
    requires nothrow_forward_output_iterator<NoThrowForwardIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    // If no throw move is requested, we will use the move constructor only if it is no throw. However, if the value 
    // type doesn't have a copy constructor, we will have to use the move constructor anyway
    if constexpr ((NoThrowMove && (std::is_nothrow_move_constructible_v<value_type> || !std::is_copy_constructible_v<value_type>))
                  || (!NoThrowMove && std::is_move_constructible_v<value_type>)) {
        return uninitialized_move(first, last, dest, allocator);
    } else {
        return uninitialized_copy(first, last, dest, allocator);
    }
}

template<typename T>
struct try_move_return {
    using type = std::conditional_t<std::is_move_assignable_v<T>, T&&, const T&>;
};


/**
 * @brief Replace the content of an object at a given address with another object
 * 
 * The given object is moved if its type has a move constructor
 * 
 * @param pos the address of the object to assigned to
 * @param value the object to assigned from
 */
template<typename T>
try_move_return<T>::type try_move(T& value) {
    return std::move(value);
}

/**
 * @brief a variant of move_safe that allows the value
 *      to be an rvalue type
 */
template<typename T>
try_move_return<T>::type try_move(T&& value) {
    return try_move(value);
}


/**
 * @brief move the elements from the range to an initialized memory area
 * 
 * If the type of the underyling data has a move assignment operator, the elements in the range are moved.
 * Otherwise, they are copy assigned
 * 
 * @tparam InputIt the type of the input iterator, needs to satisfy the std::input_iterator concept
 * @tparam OutputIt the type of the output iterator, needs to satisfy the nothrow_forward_output_iterator concept
 * @param first an iterator pointing at the beginning of the range to move from
 * @param last an iterator pointing at one past the last element in the range to move from
 * @param dest the output iterator to move to
 */
template<bool NoThrowMove, std::input_iterator InputIt, typename OutputIt>
OutputIt try_move(InputIt first, InputIt last, OutputIt dest) 
    requires nothrow_forward_output_iterator<OutputIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    // If no throw move is requested, we will use the move assignment operator only if it is no throw. However, if the 
    // value type doesn't have a copy assignment operator, we will have to use the move assignment operator anyway
    if constexpr (is_memcpyable_v<OutputIt, InputIt>) {
        std::ptrdiff_t count = last - first;
        std::memmove(dest, first, count * sizeof(value_type));
        return dest + count;
    } else if constexpr ((NoThrowMove && (std::is_nothrow_move_assignable_v<value_type> || !std::is_copy_assignable_v<value_type>))
                  || (!NoThrowMove && std::is_move_assignable_v<value_type>)) {
        return std::move(first, last, dest);
    } else {
        // If a nothrow move assignment operator doesn't exist, fall back to copy assignment operator
        return std::copy(first, last, dest);
    }
}

/**
 * @brief move the elements from the range to an initialized memory area
 * 
 * If the type of the underyling data has a move assignment operator, the elements in the range are moved.
 * Otherwise, they are copy assigned
 * 
 * @tparam InputIt the type of the input iterator, needs to satisfy the std::bidirectional_iterator concept
 * @tparam OutputIt the type of the output iterator, needs to satisfy the nothrow_bidirectional_output_iterator concept
 * @param first an iterator pointing at the beginning of the range to move from
 * @param last an iterator pointing at one past the last element in the range to move from
 * @param dest the output iterator to move tos
 */
template<bool NoThrowMove, std::bidirectional_iterator InputIt, typename OutputIt>
OutputIt try_move_backwards(InputIt first, InputIt last, OutputIt d_last) 
    requires nothrow_bidirectional_output_iterator<OutputIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    // If no throw move is requested, we will use the move assignment operator only if it is no throw. However, if the 
    // value type doesn't have a copy assignment operator, we will have to use the move assignment operator anyway
    if constexpr (is_memcpyable_v<OutputIt, InputIt>) {
        std::ptrdiff_t count = last - first;
        OutputIt dest = d_last - count;
        std::memmove(dest, first, count * sizeof(value_type));
        return dest;
    } else if constexpr ((NoThrowMove && (std::is_nothrow_move_assignable_v<value_type> || !std::is_copy_assignable_v<value_type>))
                         || (!NoThrowMove && std::is_move_assignable_v<value_type>)) {
        return std::move_backward(first, last, d_last);
    } else {
        // If a nothrow move assignment operator doesn't exist, fall back to copy assignment operator
        return std::copy_backward(first, last, d_last);
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
template<bool NoThrowMove, std::input_iterator InputIt, typename Allocator>
std::iter_value_t<InputIt>* try_move_construct(InputIt start, InputIt end, 
                                               std::size_t dest_size, Allocator& allocator) {
    using T = std::iter_value_t<InputIt>;
    T* dest = std::allocator_traits<Allocator>::allocate(allocator, dest_size);
    std::unique_ptr<T, deleter<T, Allocator> > cleaner(dest, deleter<T, Allocator>(dest_size, allocator));
    try_uninitialized_move<NoThrowMove>(start, end, dest, allocator);
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
    const T& operator()(const std::pair<const T, U>& pair) const noexcept {
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
    bool operator()(__attribute__((unused)) const T& value1, __attribute__((unused)) const T& value2) const noexcept {
        return choose_first;
    }
};

template<typename Container>
bool container_equals(const Container& container1, const Container& container2) requires equality_comparable<typename Container::value_type> {
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
std::strong_ordering container_three_way_comparison(const Container& container1, const Container& container2) requires less_comparable<typename Container::value_type> {
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
