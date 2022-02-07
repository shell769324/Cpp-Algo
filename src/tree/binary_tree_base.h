#pragma once
#include "binary_tree_node.h"
#include "binary_tree_common.h"

namespace algo {

/**
 * @brief Base class for all binary tree classes
 * 
 * @tparam K the type of the key
 * @tparam V the type of the value
 * @tparam KeyOf the key extractor function or functor
 * @tparam Comparator the key comparator function of functor
 */
template <typename K, typename V, typename KeyOf, typename Comparator = std::less<K> >
    requires binary_tree_definable<K, V, KeyOf, Comparator>
class binary_tree_base {
protected:
    KeyOf key_of;
    Comparator comp;

    /**
     * @brief Compare two keys
     * @return -1 if k1 < k2,
     *         0 if k1 == k2,
     *         1 if k1 > k2
     */
    int key_comp(const K& k1, const K& k2) const noexcept {
        if (comp(k1, k2)) {
            return -1;
        }
        return comp(k2, k1) ? 1 : 0;
    }

public:
    binary_tree_base() = default;

    binary_tree_base(const Comparator& comp) : comp(comp) { }

    binary_tree_base(Comparator&& comp) : comp(std::move(comp)) { }
};

}