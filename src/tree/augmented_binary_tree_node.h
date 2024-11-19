#include "binary_tree_common.h"

#include "binary_tree_node.h"
#include <utility>

namespace algo {

/**
 * @brief CRTP base class for binary tree node
 * 
 * The node has a parent pointer to simplify iterator implementation
 * 
 * @tparam T the type of the value of the node
 * @tparam Derived the type of derived binary tree node
 */
template <typename T, typename V, typename Derived>
class augmented_binary_tree_node_base : public binary_tree_node_base<T, Derived> {
public:
    using parent_type = binary_tree_node_base<T, Derived>;

    V augmented_value;


    /**
     * @brief Construct a new binary tree node base object
     * 
     * Value is default constructued
     * All child and parent pointers are null initialized
     */
    augmented_binary_tree_node_base() requires std::default_initializable<T> && std::default_initializable<V> : parent_type(nullptr) { }

    /**
     * @brief Construct a new binary tree node base object with 
     *        arguments to construct the value
     * 
     * @tparam Args the arguments to construct the value
     */
    template <typename... Args1, typename... Args2>
    requires (!singleton_pack_decayable_to<binary_tree_node_base, Args...>)
    augmented_binary_tree_node_base(std::piecewise_construct, std::tuple<Args1...> first_args, std::tuple<Args2...> second_args) : 
        parent_type(std::forward<Args1>(first_args)...), augmented_value(std::forward<Args2>(second_args)...) { }

    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    augmented_binary_tree_node_base(const augmented_binary_tree_node_base& other) = delete;
    augmented_binary_tree_node_base(augmented_binary_tree_node_base&& other) = delete;

    /**
     * @brief virtual destructor
     */
    virtual ~augmented_binary_tree_node_base() = default;
};
}
