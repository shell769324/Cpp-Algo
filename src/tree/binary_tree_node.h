#pragma once
#include "src/common.h"


namespace algo {
/**
 * @brief A deleter that does nothing so that compiler doesn't try to instantiate deleted node destructors
 * 
 * @tparam T the type of elements to delete
 */
template<typename T>
struct stub_deleter {
    void operator()(T*) { }
};

/**
 * @brief CRTP base class for binary tree node
 * 
 * The node has a parent pointer to simplify iterator implementation
 * 
 * @tparam T the type of the value of the node
 * @tparam Derived the type of derived binary tree node
 */
template <typename T, typename Derived>
class binary_tree_node_base {
public:
    using value_type = T;

    using unique_ptr_type = std::unique_ptr<Derived, stub_deleter<Derived> >;
    value_type value;
    // Take ownership of child pointers
    unique_ptr_type left_child;
    unique_ptr_type right_child;
    // Parent pointer is only for observation
    Derived* parent;

    // We should never construct binary_tree_node_base through constructor
    // Doing so will inevitably involves some form of constructor of iterable_node, 
    // and some form of constructor of T will be invoked
    // We want to use allocator to construct all objects of type T
    binary_tree_node_base() = delete;

    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    binary_tree_node_base(const binary_tree_node_base& other) = delete;
    binary_tree_node_base(binary_tree_node_base&& other) = delete;

    /**
     * @brief Never use the destructor since it will destroy the value
     * 
     * We rely on node_deleter to destroy the value with allocator_triats
     */
    ~binary_tree_node_base() = delete;

    /**
     * @brief Get a reference to the underlying node type
     */
    Derived& underlying_ref() noexcept {
        return static_cast<Derived&>(*this);
    }

    /**
     * @brief Get a constant reference to the underlying node type
     */
    const Derived& underlying_const_ref() const noexcept {
        return static_cast<const Derived&>(*this);
    }

    /**
     * @brief Get a pointer to the underlying node type
     */
    Derived* underlying_ptr() noexcept {
        return static_cast<Derived*>(this);
    }

    /**
     * @brief Get a constant pointer to the underlying node type
     */
    const Derived* underlying_const_ptr() const noexcept {
        return static_cast<const Derived*>(this);
    }

    /**
     * @brief Test if this node has no parent
     */
    bool is_root() const noexcept {
        return !parent;
    }

    /**
     * @brief Test if this node has no child
     */
    bool is_leaf() const noexcept {
        return !left_child && !right_child;
    }

    /**
     * @brief Test if this node is a left_child
     */
    bool is_left_child() const noexcept {
        return parent -> left_child.get() == this;
    }

    /**
     * @brief Remove its left child if it exists
     * 
     * @tparam NullSafe true if this node may not have a left child
     * @return the pointer to the left child if it exists
     *         nullptr if it doesn't have left child
     */
    template <bool NullSafe=true>
    Derived* orphan_left_child() noexcept {
        if constexpr (NullSafe) {
            if (!left_child) {
                return nullptr;
            }
        }
        Derived* old_child = left_child.release();
        old_child -> parent = nullptr;
        return old_child;
    }

    /**
     * @brief Remove its right child if it exists
     * 
     * @tparam NullSafe true if this node may not have a right child
     * @return the pointer to the right child if it exists
     *         nullptr if it doesn't
     */
    template <bool NullSafe=true>
    Derived* orphan_right_child() noexcept {
        if constexpr (NullSafe) {
            if (!right_child) {
                return nullptr;
            }
        }
        Derived* old_child = right_child.release();
        old_child -> parent = nullptr;
        return old_child;
    }

    /**
     * @brief Make a nonnull node its left child
     * 
     * Release ownership to the old left child
     * 
     * @param child a unique ptr to the new left child. Must be nonnull
     * @return the old left child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_left_child(unique_ptr_type&& child) noexcept {
        Derived* old_child = orphan_left_child();
        left_child = std::move(child);
        left_child -> parent = underlying_ptr();
        return old_child;
    }

    /**
     * @brief Make a node its left child
     * 
     * Release ownership to the old left child
     * 
     * @param child a unique ptr to the new left child. Can be null.
     * @return the old left child if it exists
     *         nullptr if it doesn't
     */
    Derived* safe_link_left_child(unique_ptr_type&& child) noexcept {
        if (child) {
            return link_left_child(std::move(child));
        }
        return orphan_left_child();
    }

    /**
     * @brief Make a nonnull node its left child
     * 
     * Release ownership to the old left child
     * 
     * @param child the new left child. Must be nonnull
     * @return the old left child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_left_child(Derived* child) noexcept {
        if (child == left_child.get()) {
            return child;
        }
        return link_left_child(unique_ptr_type(child, left_child.get_deleter()));
    }

    /**
     * @brief Make a node its left child
     * 
     * Release ownership to the old left child
     * 
     * @param child the new left child. This child can be null
     * @return the old left child if it exists
     *         nullptr if it doesn't
     */
    Derived* safe_link_left_child(Derived* child) noexcept {
        if (child) {
            return link_left_child(child);
        }
        return orphan_left_child();
    }

    /**
     * @brief Make a nonnull node its right child
     * 
     * Release ownership to the old right child
     * 
     * @param child a unique ptr to the new right child. Must be nonnull
     * @return the old right child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_right_child(unique_ptr_type&& child) noexcept {
        Derived* old_child = orphan_right_child();
        right_child = std::move(child);
        right_child -> parent = underlying_ptr();
        return old_child;
    }

    /**
     * @brief Make a node its right child
     * 
     * Release ownership to the old right child
     * 
     * @param child a unique ptr to the new right child. Can be null
     * @return the old right child if it exists
     *         nullptr if it doesn't
     */
    Derived* safe_link_right_child(unique_ptr_type&& child) noexcept {
        if (child) {
            return link_right_child(std::move(child));
        }
        return orphan_right_child();
    }

    /**
     * @brief Make a nonnull node its right child
     * 
     * Release ownership to the old right child
     * 
     * @param child the new right child. Must be nonnull
     * @return the old right child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_right_child(Derived* child) noexcept {
        if (child == right_child.get()) {
            return child;
        }
        return link_right_child(unique_ptr_type(child, right_child.get_deleter()));
    }

    /**
     * @brief Make a node its right child
     * 
     * Release ownership to the old right child
     * 
     * @param child the new right child. This child can be null
     * @return the old right child if it exists
     *         nullptr if it doesn't
     */
    Derived* safe_link_right_child(Derived* child) noexcept {
        if (child) {
            return link_right_child(child);
        }
        return orphan_right_child();
    }

    /**
     * @brief Set its left or right child, depending on a boolean flag
     * 
     * Release ownership to the old child
     * 
     * @param child a unique pointer to the new child. Must be nonnull
     * @param is_left_child setting left child if true, right child otherwise
     * @return the old child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_child(unique_ptr_type&& child, bool is_left_child) noexcept {
        if (is_left_child) {
            return link_left_child(std::move(child));
        }
        return link_right_child(std::move(child));
    }

    /**
     * @brief Set its left or right child, depending on a boolean flag
     * 
     * Release ownership to the old child
     * 
     * @param child a unique pointer to the new child. Can be null
     * @param is_left_child setting left child if true, right child otherwise
     * @return the old child if it exists
     *         nullptr if it doesn't
     */
    Derived* safe_link_child(unique_ptr_type&& child, bool is_left_child) noexcept {
        if (!child) {
            if (is_left_child) {
                return orphan_left_child();
            }
            return orphan_right_child();
        }
        return link_child(std::move(child), is_left_child);
    }

    /**
     * @brief Set its left or right child, depending on a boolean flag
     * 
     * Release ownership to the old child
     * 
     * @param child the old child. Must be nonnull
     * @param is_left_child setting left child if true, right child otherwise
     * @return the old child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_child(Derived* child, bool is_left_child) noexcept {
        if (is_left_child) {
            return link_left_child(child);
        }
        return link_right_child(child);
    }

    /**
     * @brief Set its left or right child, depending on a boolean flag
     * 
     * Release ownership to the old child
     * 
     * @param child the old child. Can be null
     * @param is_left_child setting left child if true, right child otherwise
     * @return the old child if it exists
     *         nullptr if it doesn't
     */
    Derived* safe_link_child(Derived* child, bool is_left_child) noexcept {
        if (!child) {
            if (is_left_child) {
                return orphan_left_child();
            }
            return orphan_right_child();
        }
        return link_child(child, is_left_child);
    }

    /**
     * @brief Let its parent disown itself
     * 
     * @return the old parent
     */
    Derived* orphan_self() noexcept {
        if (is_left_child()) {
            parent -> left_child.release();
        } else {
            parent -> right_child.release();
        }
        Derived* saved_parent = parent;
        parent = nullptr;
        return saved_parent;
    }

    Derived* get_root() noexcept {
        Derived* curr = underlying_ptr();
        while (curr -> parent) {
            curr = curr -> parent;
        }
        return curr;
    }

    /**
     * @brief Get the leftmost descendant of this node
     */
    Derived* get_leftmost_descendant() noexcept {
        Derived* curr = underlying_ptr();
        while (curr -> left_child) {
            curr = curr -> left_child.get();
        }
        return curr;
    }

    /**
     * @brief Get the rightmost descendant of this node
     */
    Derived* get_rightmost_descendant() noexcept {
        Derived* curr = underlying_ptr();
        while (curr -> right_child) {
            curr = curr -> right_child.get();
        }
        return curr;
    }

public:
    /**
     * @brief Get the successor of this node
     */
    Derived* next() const noexcept {
        const Derived* curr = underlying_const_ptr();
        if (curr -> right_child) {
            return curr -> right_child -> get_leftmost_descendant();
        }
        while (curr -> parent && curr -> parent -> right_child.get() == curr) {
            curr = curr -> parent;
        }
        return curr -> parent;
    }

    /**
     * @brief Get the predecessor of this node
     */
    Derived* prev() const noexcept {
        const Derived* curr = underlying_const_ptr();
        if (curr -> left_child) {
            return curr -> left_child -> get_rightmost_descendant();
        }
        while (curr -> parent && curr -> parent -> left_child.get() == curr) {
            curr = curr -> parent;
        }
        return curr -> parent;
    }

    /**
     * @brief Perform a left rotation on the current node
     * 
     * @return the node in place of this node
     */
    Derived* rotate_left() noexcept {
        /*
         *     P               P
         *     |(1)            |(1)
         *     B               D
         *    / \(2)       (2)/ \
         *   A   D     ->    B   E
         *   (3)/ \         / \(3)
         *     C   E       A   C
         */
        Derived* original_right_child = orphan_right_child<false>();
        Derived* right_child_left_child = original_right_child -> left_child.release();
        // (1) Let right child be the new child of the parent of this node
        // Ownership transfer. Parent releases ownership of pointer to this node
        if (parent) {
            parent -> link_child(original_right_child, is_left_child());
        }
        // (2) Make this node the left child of the original right child
        original_right_child -> link_left_child(underlying_ptr());
        // (3) If the original right child has a left child, 
        // make it the right child of this node
        if (right_child_left_child) {
            link_right_child(right_child_left_child);
        }
        return original_right_child;
    }

    /**
     * @brief Perform a right rotation on the current node
     * 
     * @return Derived* the node in place of this node
     */
    Derived* rotate_right() noexcept {
        /*
         *      P               P
         *      |(1)            |(1)
         *      D               B
         *  (2)/ \             / \(2)
         *    B   E     ->    A   D
         *   / \(3)           (3)/ \
         *  A   C               C   E
         */
        Derived* original_left_child = orphan_left_child<false>();
        Derived* left_child_right_child = original_left_child -> right_child.release();
        // (1) Let right child be the new child of the parent of this node
        // Ownership transfer. Parent releases ownership of pointer to this node
        if (parent) {
            parent -> link_child(original_left_child, is_left_child());
        }
        // (2) Make this node the left child of the original right child
        original_left_child -> link_right_child(underlying_ptr());
        // (3) If the original right child has a left child, 
        // make it the right child of this node
        if (left_child_right_child) {
            link_left_child(left_child_right_child);
        }
        return original_left_child;
    }

    template <typename Allocator, typename... Args>
    requires std::same_as<Derived, typename Allocator::value_type>
    static Derived* construct(Allocator& allocator, Args&&... args) {
        using alloc_traits = std::allocator_traits<Allocator>;
        Derived* node = alloc_traits::allocate(allocator, 1);
        try {
            alloc_traits::construct(allocator, &node -> value, std::forward<Args>(args)...);
        } catch (...) {
            alloc_traits::deallocate(allocator, node, 1);
            throw;
        }
        node -> parent = nullptr;
        new(&node -> left_child) unique_ptr_type;
        new(&node -> right_child) unique_ptr_type;
        return node;
    }

    template <typename Allocator>
    requires std::same_as<Derived, typename Allocator::value_type>
    static Derived* construct_sentinel(Allocator& allocator) {
        using alloc_traits = std::allocator_traits<Allocator>;
        Derived* node = alloc_traits::allocate(allocator, 1);
        node -> parent = nullptr;
        new(&node -> left_child) unique_ptr_type;
        new(&node -> right_child) unique_ptr_type;
        return node;
    }

    /**
     * @brief Make a shallow clone by only copying the value
     * 
     * Parent pointer and child pointers are not copied
     * 
     * @return a clone of this node
     */
    template <typename Allocator>
    requires std::same_as<Derived, typename Allocator::value_type>
    Derived* clone(Allocator& allocator) const {
        return Derived::construct(allocator, value);
    }
    
    /**
     * @brief Make a deep clone by iteratively cloning all children
     * 
     * Only copy self and its descendants
     * 
     * @return a deep clone of this node
     */
    template <typename Allocator>
    requires std::same_as<Derived, typename Allocator::value_type>
    Derived* deep_clone(Allocator& allocator) const {
        Derived* clone_root = underlying_const_ptr() -> clone(allocator);
        const Derived* original_curr = underlying_const_ptr();
        Derived* clone_curr = clone_root;
        // We will deep clone this node by doing iterative preorder traversal
        // Loop guard is inside the loop
        while (true) {
            // If the left child is not nullptr,
            // clone it and move to the new left child
            if (original_curr -> left_child) {
                clone_curr -> link_left_child(original_curr -> left_child -> clone(allocator));
                original_curr = original_curr -> left_child.get();
                clone_curr = clone_curr -> left_child.get();
            } else {
                /* If the left child is nullptr, we have two cases
                 * 1. The current node has a right child:
                 *    We will clone the right child and move to it
                 * 2. The right child is also nullptr:
                 *    We will move up until we see a node with uncloned right child.
                 *    Note that all left children along the way have already been cloned
                 *    because we are doing preorder traversal. We should ignore them when
                 *    deciding when we want to stop
                 * 
                 * For case 2, there are three cases we want to discriminate as we move up
                 * 1. The original has a right child and the clone has a right child:
                 *        . <- original
                 *       / \
                 *     [.]  . <- original in the last iteration
                 *        . <- clone
                 *       / \
                 *     [.]  . <- clone in the last iteration
                 *    We have moved from a right child to its parent. The right child has been cloned
                 *    so we should keep moving up
                 * 
                 * 2. The original has no right child and the clone has no right child:
                 *        . <- original
                 *       /
                 *      . <- original in the last iteration
                 *        . <- clone
                 *       /
                 *      . <- clone in the last iteration
                 *    We have moved from a left child to its parent. Since the original has no right child
                 *    we should keep moving up
                 * 
                 * 3. The current has a right child and the clone has no right child:
                 *                                         . <- original
                 *                                        / \
                 *     original in the last iteration -> .   . <- TO BE CLONED
                 *                                         . <- node
                 *                                        /
                 *        clone in the last iteration -> .
                 *    We have moved from a left child to its parent. The original has a right child but the clone
                 *    doesn't. Now we clone the right child and move to it.
                 */
                while (original_curr != parent && !(original_curr -> right_child && !clone_curr -> right_child)) {
                    original_curr = original_curr -> parent;
                    clone_curr = clone_curr -> parent;
                }
                /* If we break the inner loop by hitting the parent of the root, we were moving up along the 
                 * right spine. The whole tree has been cloned and we should break the outer loop as well.
                 */
                if (original_curr == parent) {
                    break;
                }
                clone_curr -> link_right_child(original_curr -> right_child -> clone(allocator));
                original_curr = original_curr -> right_child.get();
                clone_curr = clone_curr -> right_child.get();
            }
        }
        return clone_root;
    }


    template <typename Allocator>
    requires std::same_as<Derived, typename Allocator::value_type>
    void destroy(Allocator& allocator) {
        using alloc_traits = std::allocator_traits<Allocator>;
        alloc_traits::destroy(allocator, &this -> value);
        alloc_traits::deallocate(allocator, underlying_ptr(), 1);
    }

    template <typename Allocator>
    requires std::same_as<Derived, typename Allocator::value_type>
    void deep_destroy(Allocator& allocator) {
        if (left_child) {
            left_child.release() -> deep_destroy(allocator);
        }
        if (right_child) {
            right_child.release() -> deep_destroy(allocator);
        }
        underlying_ptr() -> destroy(allocator);
    }

    /**
     * @brief Check if all nodes of in this tree have their parent/children pointers set correctly
     * 
     * @param node the root of the tree
     * @return true iff all nodes and their children are doubly linked
     */
    bool __is_parent_child_link_mutual() const noexcept {
        if (left_child) {
            left_child -> __is_parent_child_link_mutual();
            if (left_child -> parent != this) {
                return false;
            }
        }
        if (right_child) {
            right_child -> __is_parent_child_link_mutual();
            if (right_child -> parent != this) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief A binary tree node
 * 
 * @tparam T the type of the value of the node
 */
template <class T>
class binary_tree_node: public binary_tree_node_base<T, binary_tree_node<T> > { };
}
