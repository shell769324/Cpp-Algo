#pragma once
#include "src/common.h"


namespace algo {
/**
 * @brief Interface for iterable binary tree node
 * 
 * @tparam T the type of the value in the node
 */
template <typename T>
class iterable_node {
public:
    T value;

private:
    /**
     * Non virtual interface (NVI) idiom because binary_tree_node_base class is a CRTP
     * class. Most of its methods return the underyling type for ease of use. If we use regular
     * approach (virtual method and override), the return type of those methods will have to be a 
     * pointer to a subclass of iterable_node (covariant return type). However, the compiler doesn't
     * know the derived class of binary_tree_node_base is actually derived class at the 
     * instantiation of iterable_node because the derived class is still incomplete. That makes
     * covariant return type impossible.
     * 
     * With NVI, we can hide the virtual method hierachy from the outside. The next and prev methods
     * of the parent and child can return unreleated types (though we know they ARE covariant).
     */

    /**
     * @brief Get the successor of this node
     */
    virtual iterable_node* do_next() const noexcept = 0;

    /**
     * @brief Get the predecessor of this node
     */
    virtual iterable_node* do_prev() const noexcept = 0;

public:
    /**
     * @brief Construct a new iterable_node object with default value
     */
    iterable_node() = default;

    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    iterable_node(const iterable_node& other) = delete;
    iterable_node(iterable_node&& other) = delete;

    /**
     * @brief Create a new iterable node given the arguments to construct a value
     * 
     * This constructor is disabled if the parameter pack is a singleton and
     * expanded to a type that decays to this class or its derived class.
     * This is intentional so the copy/move constructors won't be overshadowed
     * 
     * @tparam Args the arguments to construct a value
     */
    template <typename... Args>
    requires (!singleton_pack_decayable_to<iterable_node, Args...>)
    iterable_node(Args&&... args) : value(std::forward<Args>(args)...) { }

    /**
     * @brief virtual destructor
     */
    virtual ~iterable_node() = default;

    /**
     * @brief Get the successor of this node
     */
    iterable_node* next() const noexcept {
        return do_next();
    }

    /**
     * @brief Get the predecessor of this node
     */
    iterable_node* prev() const noexcept {
        return do_prev();
    }
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
class binary_tree_node_base : public iterable_node<T> {
public:
    // Take ownership of child pointers
    std::unique_ptr<Derived> left_child;
    std::unique_ptr<Derived> right_child;
    // Parent pointer is only for observation
    Derived* parent;

    /**
     * @brief Construct a new binary tree node base object
     * 
     * Value is default constructued
     * All child and parent pointers are null initialized
     */
    binary_tree_node_base() requires std::default_initializable<T> : parent(nullptr) { }

    template <typename... Args>
    requires (!singleton_pack_decayable_to<binary_tree_node_base, Args...>)
    binary_tree_node_base(Args&&... args) : iterable_node<T>(std::forward<Args>(args)...), parent(nullptr) { }

    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    binary_tree_node_base(const binary_tree_node_base& other) = delete;
    binary_tree_node_base(binary_tree_node_base&& other) = delete;

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
     * @brief Test if a node has no parent
     */
    bool is_root() const noexcept {
        return !parent;
    }

    /**
     * @brief Test if a node has no child
     */
    bool is_leaf() const noexcept {
        return !left_child && !right_child;
    }

    /**
     * @brief Remove its left child if it exists
     * 
     * @return the pointer to the left child if it exists
     *         nullptr if it doesn't have left child
     */
    Derived* orphan_left_child() noexcept {
        if (!left_child) {
            return nullptr;
        }
        Derived* old_child = left_child.release();
        old_child -> parent = nullptr;
        return old_child;
    }

    /**
     * @brief Remove its right child if it exists
     * 
     * @return the pointer to the right child if it exists
     *         nullptr if it doesn't
     */
    Derived* orphan_right_child() noexcept {
        if (!right_child) {
            return nullptr;
        }
        Derived* old_child = right_child.release();
        old_child -> parent = nullptr;
        return old_child;
    }

    /**
     * @brief Set its left child
     * 
     * Release ownership to the old left child
     * 
     * @param child a unique ptr to the new left child
     * @return the old left child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_left_child(std::unique_ptr<Derived>&& child) noexcept {
        Derived* old_child = orphan_left_child();
        left_child = std::move(child);
        left_child -> parent = underlying_ptr();
        return old_child;
    }

    /**
     * @brief Set its left child
     * 
     * Release ownership to the old left child
     * 
     * @param child the new left child
     * @return the old left child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_left_child(Derived* child) noexcept {
        return link_left_child(std::unique_ptr<Derived>(child));
    }

    /**
     * @brief Set its right child
     * 
     * Release ownership to the old right child
     * 
     * @param child a unique ptr to the new right child
     * @return the old right child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_right_child(std::unique_ptr<Derived>&& child) noexcept {
        Derived* old_child = orphan_right_child();
        right_child = std::move(child);
        right_child -> parent = underlying_ptr();
        return old_child;
    }

    /**
     * @brief Set its right child
     * 
     * Release ownership to the old right child
     * 
     * @param child the new right child
     * @return the old right child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_right_child(Derived* child) noexcept {
        return link_right_child(std::unique_ptr<Derived>(child));
    }

    /**
     * @brief Set its left or right child, depending on a boolean flag
     * 
     * Release ownership to the old child
     * 
     * @param child a unique pointer to the new child
     * @param is_left_child setting left child if true, right child otherwise
     * @return the old child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_child(std::unique_ptr<Derived>&& child, bool is_left_child) noexcept {
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
     * @param child the old child
     * @param is_left_child setting left child if true, right child otherwise
     * @return the old child if it exists
     *         nullptr if it doesn't
     */
    Derived* link_child(Derived* child, bool is_left_child) noexcept {
        return link_child(std::unique_ptr<Derived>(child), is_left_child);
    }

    /**
     * @brief Let its parent disown itself
     * 
     * @return the old parent
     */
    Derived* orphan_self() noexcept {
        if (parent -> left_child.get() == this) {
            parent -> left_child.release();
        } else {
            parent -> right_child.release();
        }
        return parent = nullptr;
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

private:
    /**
     * NVI private overriding methods
     */

    /**
     * @brief Get the successor of this node
     */
    binary_tree_node_base* do_next() const noexcept override {
        return next();
    }

    /**
     * @brief Get the predecessor of this node
     */
    binary_tree_node_base* do_prev() const noexcept override {
        return prev();
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
     * @brief Make a shallow clone by only copying the value
     * 
     * Parent pointer and child pointers are not copied
     * 
     * @return a clone of this node
     */
    Derived* clone() const {
        return underlying_const_ref().clone();
    }
    
    /**
     * @brief Make a deep clone by iteratively cloning all children
     * 
     * Only copy self and its descendants
     * 
     * @return a deep clone of this node
     */
    Derived* deep_clone() const {
        Derived* clone_root = clone();
        const Derived* original_curr = underlying_const_ptr();
        Derived* clone_curr = clone_root;
        // We will deep clone this node by doing iterative preorder traversal
        // Loop guard is inside the loop
        while (true) {
            // If the left child is not nullptr,
            // clone it and move to the new left child
            if (original_curr -> left_child) {
                clone_curr -> link_left_child(original_curr -> left_child -> clone());
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
                clone_curr -> link_right_child(original_curr -> right_child -> clone());
                original_curr = original_curr -> right_child.get();
                clone_curr = clone_curr -> right_child.get();
            }
        }
        return clone_root;
    }
};

/**
 * @brief A binary tree node
 * 
 * @tparam T the type of the value of the node
 */
template <class T>
class binary_tree_node: public binary_tree_node_base<T, binary_tree_node<T> > {
public:
    using binary_tree_node_base<T, binary_tree_node<T> >::binary_tree_node_base;
    binary_tree_node* clone() const {
        return new binary_tree_node(this -> value);
    }
};
}