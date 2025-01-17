#pragma once
#include <concepts>
#include <iterator>

namespace algo {

template<typename T>
concept copy_or_move_assignable = std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>;

/**
 * @brief nothrow_forward_output_iterator as required by a few methods like
 *        uninitialized_move, move, uninitialized_copy, copy etc
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

/**
 * @brief nothrow_bidirectional_output_iterator as required by a few methods like
 *        uninitialized_move_backwards, uninitialized_copy_backwards, copy etc
 * 
 * @tparam I the type of the iterator
 * @tparam T the type of the underlying data
 */
template<typename I, typename T>
concept nothrow_bidirectional_output_iterator =
    std::bidirectional_iterator<I> && (std::indirectly_writable<I, T&> || std::indirectly_writable<I, T&&>) && requires(I& it) {
        { *it } noexcept -> std::convertible_to<T&>;
        { it++ } noexcept;
        { ++it } noexcept;
        { it-- } noexcept;
        { --it } noexcept;
    } && requires(I it1, I it2) {
        { it1 == it2 } noexcept;
    };

// If the iterator doesn't contain a type that equals to the value type after stripped of cv qualification
// we can't really tell much. However it does, there must be a compatible copy/move constructor of the value type
// that works with the reference type of the iterator
template<typename I, typename T>
concept conjugate_uninitialized_iterator =
    (!std::same_as<std::remove_cv_t<typename std::iterator_traits<I>::value_type>, std::remove_cv_t<T> >)
    || std::is_copy_constructible_v<T>
    || (std::is_rvalue_reference_v<typename std::iterator_traits<I>::reference> && std::is_move_constructible_v<T>);

template<typename I, typename T>
concept conjugate_iterator =
    (!std::same_as<std::remove_cv_t<typename std::iterator_traits<I>::value_type>, std::remove_cv_t<T> >)
    || std::is_copy_assignable_v<T>
    || (std::is_rvalue_reference_v<typename std::iterator_traits<I>::reference> && std::is_move_assignable_v<T>);

template<typename I, typename T>
concept conjugate_uninitialized_input_iterator =
    std::input_iterator<I> && conjugate_uninitialized_iterator<I, T>;

template<typename I, typename T>
concept conjugate_uninitialized_forward_iterator =
    std::forward_iterator<I> && conjugate_uninitialized_iterator<I, T>;

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

}