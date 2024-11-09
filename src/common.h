#pragma once

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
 */
template<typename T>
struct deleter {
    void operator()(T* memory) {
        ::operator delete(memory);
    };
};

template<typename T>
class safe_move_construct_deleter {
private:
    std::size_t length;
public:
    safe_move_construct_deleter(std::size_t length) : length(length) { }

    void operator()(T* memory) {
        std::destroy(memory, memory + length);
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
void safe_uninitialized_move(T* pos, T& value) {
    if constexpr (std::is_nothrow_move_constructible_v<T>) {
        new(pos) T(std::move(value));
    } else {
        new(pos) T(value);
    }
}

/**
 * @brief a variant of uninitialized_move_safe that allows the value
 *      to be an rvalue type
 */
template<typename T>
void safe_uninitialized_move(T* pos, T&& value) {
    // Call the lvalue version
    safe_uninitialized_move(pos, value);
}

/**
 * @brief move the elements from the range to an uninitialized memory area
 * 
 * If the type of the underyling data has a nothrow move constructor, the elements in the range are moved.
 * Otherwise, they are copied
 * 
 * @tparam InputIt the type of the input iterator, needs to satisfy the std::input_iterator concept
 * @tparam OutputIt the output of the input iterator, needs to satisfy the nothrow_forward_output_iterator concept
 * @param first an iterator pointing at the beginning of the range to move from
 * @param last an iterator pointing at one past the last element in the range to move from
 * @param dest the output iterator to move to
 */
template<std::input_iterator InputIt, typename OutputIt>
void safe_uninitialized_move(InputIt first, InputIt last, OutputIt dest)
    requires nothrow_forward_output_iterator<OutputIt, std::iter_value_t<InputIt> > {
    using value_type = std::iter_value_t<InputIt>;
    if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
        std::uninitialized_move(first, last, dest);
    } else {
        // If a nothrow move constructor doesn't exist, fall back to copy constructor
        std::uninitialized_copy(first, last, dest);
    }
}



/**
 * @brief Replace the content of an object at a given address with another object
 * 
 * The given object is moved if its type has a nothrow move constructor
 * 
 * @param pos the address of the object to assigned to
 * @param value the object to assigned from
 */
template<typename T>
void safe_move(T* pos, T& value) {
    if constexpr (std::is_nothrow_move_assignable_v<T>) {
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
void safe_move(T* pos, T&& value) {
    // Call the lvalue version
    safe_move(pos, value);
}


/**
 * @brief move the elements from the range to an initialized memory area
 * 
 * If the type of the underyling data has a nothrow move assignment operator, the elements in the range are moved.
 * Otherwise, they are copy assigned
 * 
 * @tparam InputIt the type of the input iterator, needs to satisfy the std::input_iterator concept
 * @tparam OutputIt the output of the input iterator, needs to satisfy the nothrow_forward_output_iterator concept
 * @param first an iterator pointing at the beginning of the range to move from
 * @param last an iterator pointing at one past the last element in the range to move from
 * @param dest the output iterator to move to
 */
template<std::input_iterator InputIt, typename OutputIt>
void safe_move(InputIt first, InputIt last, OutputIt dest) 
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
 * @brief Create an array of a given size with the same elements as in the
 * range
 * 
 * If the type of the value has a nothrow move constructor, the elements in the
 * range will be moved. Otherwise, they are copied.
 * 
 * @param start the beginning of the range
 * @param end the end of the range
 * @param dest_size the size of the clone
 * @return pointer the clone
 */
template<std::input_iterator InputIt>
std::iter_value_t<InputIt>* safe_move_construct(InputIt start, InputIt end, std::size_t dest_size) {
    using T = std::iter_value_t<InputIt>;
    T* dest = static_cast<T*>(::operator new(sizeof(T) * dest_size));
    std::unique_ptr<T, deleter<T> > cleaner(dest);
    safe_uninitialized_move(start, end, dest);
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
}
