#pragma once
#include <concepts>

namespace algo {

/**
 * @brief Constraint on the key extractor function or functor
 * 
 * The key extractor must produce key of specified type given a value
 * of the specified type
 * 
 * @tparam K the type of the key
 * @tparam V the type of the value
 * @tparam KeyOf the key extractor function or functor
 */
template<typename K, typename V, typename KeyOf>
concept key_extractable =
    std::regular_invocable<KeyOf, V> &&
    std::same_as<K, std::decay_t<std::invoke_result_t<KeyOf, V> > >;

/**
 * @brief Constraint on a comparison-based binary tree
 * 
 * @tparam K the type of the key
 * @tparam V the type of the value
 * @tparam KeyOf the key extractor function or functor
 * @tparam Comparator the comparator function or functor
 * @tparam Allocator the allocator that allocates and deallocates raw memory
 */
template <typename K, typename V, typename KeyOf, typename Comparator, typename Allocator>
concept binary_tree_definable = key_extractable<K, V, KeyOf> && std::predicate<Comparator, K, K> 
    && std::same_as<V, typename Allocator::value_type>;

template <typename V, typename Resolver>
concept is_resolver = std::is_invocable_r_v<bool, Resolver, const V&, const V&>;

}
