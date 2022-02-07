#pragma once
#include <functional>
#include <concepts>
#include "binary_tree_common.h"
#include "binary_tree_base.h"
#include "binary_tree_node.h"
#include "binary_tree_iterator.h"
#include "src/common.h"

namespace algo {

template <typename T>
class avl_node : public binary_tree_node_base<T, avl_node<T> > {
public:

    using parent_type = binary_tree_node_base<T, avl_node<T> >;

    // balance factor is defined as the height of left child minus
    // the height of right child
    // Note that it is possible to have an implementation with only 2 bits
    // for this balance factor because it should always be -1, 0 or 1 but
    // making it a byte allows some simplification in implementing rotations
    signed char factor;

    /**
     * @brief Create a new avl node
     */
    avl_node() requires std::default_initializable<T> : factor(0) {}
    
    /**
     * @brief Construct a new iterable_node object
     * 
     * @param value used to copy construct the value of this node
     */
    template <typename... Args>
    requires (!singleton_pack_decayable_to<avl_node, Args...>)
    avl_node(Args&&... args) : parent_type(std::forward<Args>(args)...), factor(0) { }
    
    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    avl_node(const avl_node& other) = delete;
    avl_node(avl_node&& other) = delete;

    /**
     * @brief Create a shallow copy of this node
     * 
     * Parent or child pointers are not moved
     * 
     * @return avl_node* a shallow copy of this node
     */
    avl_node* clone() const {
        avl_node* node_clone = new avl_node(this -> value);
        node_clone -> factor = factor;
        return node_clone;
    }

    /**
     * @brief Perform a left rotation on the current node
     * 
     * The parent's balance factor won't be changed
     * 
     * @return avl_node* the node in place of this node
     */
    avl_node* rotate_left() noexcept {
        /*
         *     P               P
         *     |(1)            |(1)
         *     B               D
         *    / \(2)       (2)/ \
         *   A   D     ->    B   E
         *   (3)/ \         / \(3)
         *     C   E       A   C
         */
        bool is_left_child = this -> parent -> left_child.get() == this;
        avl_node* original_right_child = this -> right_child.release();
        avl_node* right_child_left_child = original_right_child -> left_child.release();
        // (1) Let right child be the new child of the parent of this node
        // Ownership transfer. Parent releases ownership of pointer to this node
        this -> parent -> link_child(original_right_child, is_left_child);
        // (2) Make this node the left child of the original right child
        original_right_child -> link_left_child(this);
        // (3) If the original right child has a left child, 
        // make it the right child of this node
        if (right_child_left_child) {
            this -> link_right_child(right_child_left_child);
        }
        /*
         * For simplicity, denote the height of node A as A. Given
         * BF(B) = A - D, BF(D) = C - E and D = max(C, E) + 1
         * we want to find the new balance factor for B, D.
         * 
         * Denote the new height of A after the rotation as A',
         *        the new balance factor of A after the rotation as BF'(A)
         * Note that BF(*) = BF'(*) where * = A, C, E
         * BF'(B) = A - C
         *        = A - (C + max(0, E - C) - max(0, E - C))
         *        = A - max(C, E) + max(0, E - C)
         *        = A - (max(C, E) + 1 - 1) + max(0, -BF(D))
         *        = A - D + 1 - min(BF(D), 0)
         *        = BF(B) - min(BF(D), 0) + 1
         *        -= min(BF(D), 0) - 1
         * 
         * BF'(D) = B' - E
         *        = max(A, C) + 1 - E
         *        = max(A - C, 0) + (C - E) + 1
         *        = max(BF'(B), 0) + BF(D) + 1
         *        += max(BF'(B), 0) + 1
         */
        factor -= std::min<signed char>(original_right_child -> factor, 0) - 1;
        original_right_child -> factor += std::max<signed char>(factor, 0) + 1;
        return original_right_child;
    }

    /**
     * @brief Perform a right rotation on the current node
     * 
     * The parent's balance factor won't be changed
     * 
     * @return avl_node* the node in place of this node
     */
    avl_node* rotate_right() noexcept {
        /*
         *      P               P
         *      |(1)            |(1)
         *      D               B
         *  (2)/ \             / \(2)
         *    B   E     ->    A   D
         *   / \(3)           (3)/ \
         *  A   C               C   E
         */
        bool is_left_child = this -> parent -> left_child.get() == this;
        avl_node* original_left_child = this -> left_child.release();
        avl_node* left_child_right_child = original_left_child -> right_child.release();
        // (1) Let right child be the new child of the parent of this node
        // Ownership transfer. Parent releases ownership of pointer to this node
        this -> parent -> link_child(original_left_child, is_left_child);
        // (2) Make this node the left child of the original right child
        original_left_child -> link_right_child(this);
        // (3) If the original right child has a left child, 
        // make it the right child of this node
        if (left_child_right_child) {
            this -> link_left_child(left_child_right_child);
        }
        /*
         * For simplicity, denote the height of node A as A. Given
         * BF(B) = A - C, BF(D) = B - E and B = max(A, C) + 1
         * we want to find the new balance factor for B, D.
         * 
         * Denote the new height of A after the rotation as A',
         *        the new balance factor of A after the rotation as BF'(A)
         * Note that BF(*) = BF'(*) where * = A, C, E
         * 
         * BF'(D) = C - E
         *        = (C + max(0, A - C) - max(0, A - C)) - E
         *        = max(C, A) - max(0, BF(B)) - E
         *        = B - 1 - max(0, BF(B)) - E
         *        = B - E - 1 - max(BF(B), 0)
         *        = BF(D) - max(BF(B)), 0) - 1
         *        -= max(BF(B), 0) + 1
         * 
         * BF'(B) = A - D'
         *        = A - (max(C, E) + 1)
         *        = A - (max(0, E - C) + C + 1)
         *        = A - C - max(E - C, 0) - 1
         *        = BF(B) + min(C - E, 0) - 1
         *        += min(BF'(D), 0) - 1
         */
        factor -= std::max<signed char>(original_left_child -> factor, 0) + 1;
        original_left_child -> factor += std::min<signed char>(factor, 0) - 1;
        return original_left_child;
    }

    /**
     * @brief given that this node has balance factor == 2
     *        perform a rebalance action to restore the constraint
     *
     * @return avl_node* the node in place of this node
     */
    avl_node* rebalance_left() noexcept {
        // If the left child is right heavy, we will need double rotation
        if (this -> left_child -> factor < 0) {
            // Although left child's balance factor may change,
            // its height will remain the same.
            // Therefore, we don't need to update the balance factor of this node
            this -> left_child -> rotate_left();
        }
        return rotate_right();
    }

    /**
     * @brief given that this node has balance factor == -2
     *      perform a rebalance action to restore the constraint
     * 
     * @return avl_node* the node in place of this node
     */
    avl_node* rebalance_right() noexcept {
        // If the right child is left heavy, we will need double rotation
        if (this -> right_child -> factor > 0) {
            // Although right child's balance factor may change,
            // its height will remain the same.
            // Therefore, we don't need to update the balance factor of this node
            this -> right_child -> rotate_right();
        }
        return rotate_left();
    }
};


/**
 * @brief An avl tree implementation
 * 
 * @tparam K the type of the key
 * @tparam V the type of the value
 * @tparam KeyOf key extractor function or functor
 * @tparam Comparator key comparactor function or functor
 */
template <typename K, typename V, typename KeyOf, typename Comparator = std::less<K> >
    requires binary_tree_definable<K, V, KeyOf, Comparator>
class avl_tree: public binary_tree_base <K, V, KeyOf, Comparator> {

public:
    using key_type = K;
    using value_type = V;
    using reference = V&;
    using const_reference = const V&;
    using iterator = binary_tree_iterator<V>;
    using const_iterator = binary_tree_iterator<const V>;
    using reverse_iterator = binary_tree_iterator<V, true>;
    using const_reverse_iterator = const binary_tree_iterator<const V, true>;
    using base_type = binary_tree_base<K, V, KeyOf, Comparator>;
    using node_type = avl_node<V>;

    using base_type::key_of;
    using base_type::comp;

private:
    std::unique_ptr<node_type> sentinel;
    std::size_t element_count;
    constexpr static const char EXISTS = 0x2;
    constexpr static const char IS_LEFT_CHILD = 0x1;

public:
    /**
     * @brief Construct an empty avl tree with default comparator
     */
    avl_tree() : sentinel(std::make_unique<node_type>()), element_count(0) { };

    /**
     * @brief Construct an empty avl tree with a given comparator
     * 
     * @param comp the key comparator used by the tree
     *             will be copy constructed
     */
    avl_tree(const Comparator& comp) :
        base_type(comp), sentinel(std::make_unique<node_type>()), element_count(0) { }

    /**
     * @brief Construct an empty avl tree with a given comparator
     * 
     * @param comp the key comparator used by the tree
     *             will be move constructed
     */
    avl_tree(Comparator&& comp) :
        base_type(std::move(comp)), sentinel(std::make_unique<node_type>()), element_count(0) { }

    /**
     * @brief Construct a copy of another avl tree
     * 
     * Copy constructor
     * 
     * @param other the tree to copy from
     */
    avl_tree(const avl_tree& other) : base_type(other),
        sentinel(other.sentinel->deep_clone()), element_count(other.element_count) { }

    /**
     * @brief Construct a copy of another avl tree
     * 
     * Move constructor
     * 
     * @param other the tree to move from
     */
    avl_tree(avl_tree&& other) = default;

    /**
     * @brief Copy assignment operator
     * 
     * @param other the tree to copy from
     * @return a reference to this tree
     */
    avl_tree& operator=(const avl_tree& other) {
        if (this == &other) {
            return *this;
        }
        avl_tree tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Move assignment operator
     * 
     * @param other the tree to move from
     * @return a reference to this tree
     */
    avl_tree& operator=(avl_tree&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief Construct a new avl tree from range
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginning of the range
     * @param last the end of the range
     */
    template<std::input_iterator InputIt>
    avl_tree(InputIt first, InputIt last) : avl_tree() {
        insert(first, last);
    }

    /**
     * @brief Test if a tree is empty
     */
    bool is_empty() const noexcept {
        return element_count == 0;
    }

    /**
     * @brief Get the number of elements in the tree
     */
    std::size_t size() const noexcept {
        return element_count;
    }

    /**
     * @brief Get an iterator to the smallest element
     */
    iterator begin() noexcept {
        return iterator(sentinel -> get_leftmost_descendant());
    }

    /**
     * @brief Get a constant iterator to the smallest element
     */
    const_iterator begin() const noexcept {
        return const_iterator(sentinel -> get_leftmost_descendant());
    }

    /**
     * @brief Get a constant iterator to the smallest element
     */
    const_iterator cbegin() const noexcept {
        return const_iterator(sentinel -> get_leftmost_descendant());
    }

    /**
     * @brief Get an iterator to one past the greatest element
     */
    iterator end() noexcept {
        return iterator(sentinel.get());
    }

    /**
     * @brief Get a constant iterator to one past the greatest element
     */
    const_iterator end() const noexcept {
        return const_iterator(sentinel.get());
    }

    /**
     * @brief Get a constant iterator to one past the greatest element
     */
    const_iterator cend() const noexcept {
        return const_iterator(sentinel.get());
    }

    /**
     * @brief Get an reverse iterator to the greatest element
     */
    reverse_iterator rbegin() noexcept {
        if (!sentinel -> left_child) {
            return reverse_iterator();
        }
        return reverse_iterator(sentinel -> left_child -> get_rightmost_descendant());
    }

    /**
     * @brief Get a constant reverse iterator to the greatest element
     */
    const_reverse_iterator rbegin() const noexcept {
        if (!sentinel -> left_child) {
            return const_reverse_iterator();
        }
        return const_reverse_iterator(sentinel -> left_child -> get_rightmost_descendant());
    }

    /**
     * @brief Get a constant reverse iterator to the greatest element
     */
    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    /**
     * @brief Get a reverse iterator to one past the smallest element
     */
    reverse_iterator rend() noexcept {
        return reverse_iterator();
    }

    /**
     * @brief Get a constant reverse iterator to one past the smallest element
     */
    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator();
    }

    /**
     * @brief Get a constant reverse iterator to one past the smallest element
     */
    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator();
    }

    /**
     * @brief Remove all elements in this tree
     */
    void clear() noexcept {
        sentinel -> left_child.reset();
    }

    /**
     * @brief Get the iterator to an element given a key
     * 
     * @param key the key to look up
     * @return an iterator to the element if it exists,
     *         the end iterator otherwise
     */
    iterator find(const key_type& key) {
        return iterator(std::as_const(*this).find(key));
    }

    /**
     * @brief Get the iterator to an element given a key
     * 
     * @param key the key to look up
     * @return a constant iterator to the element if it exists,
     *         the end iterator otherwise
     */
    const_iterator find(const key_type& key) const {
        node_type* curr = sentinel -> left_child.get();
        while (curr != nullptr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return const_iterator(curr);
            }
            if (result < 0) {
                curr = curr -> left_child.get();
            } else {
                curr = curr -> right_child.get();
            }
        }
        return const_iterator(sentinel.get());
    }

    /**
     * @brief Get the iterator to the greatest element less than or equal to
     *        a given key
     * 
     * @param key the inclusive upper bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    iterator max_leq(const key_type& key) {
        return iterator(std::as_const(*this).max_leq(key));
    }

    /**
     * @brief Get the iterator to the greatest element less than or equal to
     *        a given key
     * 
     * @param key the inclusive upper bound
     * @return a const iterator to such element if it exists,
     *         the end iterator otherwise
     */
    const_iterator max_leq(const key_type& key) const {
        node_type* curr = sentinel -> left_child.get();
        node_type* res = sentinel.get();
        while (curr != nullptr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return const_iterator(curr);
            }
            if (result < 0) {
                curr = curr -> left_child.get();
            } else {
                res = curr;
                curr = curr -> right_child.get();
            }
        }
        return const_iterator(res);
    }

    /**
     * @brief Get the iterator to the smallest element greater than or equal to
     *        a given key
     * 
     * @param key the inclusive lower bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    iterator min_geq(const key_type& key) {
        return iterator(std::as_const(*this).min_geq(key));
    }

    /**
     * @brief Get the iterator to the smallest element greater than or equal to
     *        a given key
     * 
     * @param key the inclusive lower bound
     * @return a const iterator to such element if it exists,
     *         the end iterator otherwise
     */
    const_iterator min_geq(const key_type& key) const {
        node_type* curr = sentinel -> left_child.get();
        node_type* res = sentinel.get();
        while (curr != nullptr) {
            int result = this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return const_iterator(curr);
            }
            if (result < 0) {
                res = curr;
                curr = curr -> left_child.get();
            } else {
                curr = curr -> right_child.get();
            }
        }
        return const_iterator(res);
    }

private:
    /**
     * @brief Get the node that would be the parent of the new node when the
     *        given key is inserted
     * 
     * No insertion is performed
     * 
     * @param key the key to insert
     * @return std::pair<node_type*, char> a pair of the node and a char flag
     * 
     * The flag has two fields, the second to the least significant bit is true if
     * the key already exists. In this case, the least significant bit will be false
     * The node will be the node that has the equal key
     * 
     * Otherwise, the least significant bit is true if the new node would be the left
     * child of the parent node. It is false otherwise. The node will be the would-be parent
     */
    std::pair<node_type*, char> get_insertion_parent(const key_type& key) const {
        node_type* curr = sentinel.get();
        while (curr) {
            int result = curr == sentinel.get() ? -1 : this -> key_comp(key, key_of(curr -> value));
            if (result == 0) {
                return std::make_pair(curr, EXISTS);
            }
            node_type* next_node;
            if (result < 0) {
                next_node = curr -> left_child.get();
                if (!next_node) {
                    return std::make_pair(curr, IS_LEFT_CHILD);
                }
            } else {
                next_node = curr -> right_child.get();
                if (!next_node) {
                    return std::make_pair(curr, 0);
                }
            }
            curr = next_node;
        }
        throw std::range_error("Impossible state");
    }

    /**
     * @brief Populate the balance factor change after this node is inserted
     * 
     * All ancestors of this node will have their balance factors updated
     * 
     * Rebalance is performed if necessary
     * 
     * @param new_node the newly inserted node
     */
    void populate_factor_after_insertion(node_type* new_node) {
        for (node_type* curr = new_node, *par = new_node -> parent; par != sentinel.get();
                curr = par, par = par -> parent) {
            if (curr == par -> left_child.get()) {
                par -> factor++;
                if (par -> factor == 2) {
                    par = par -> rebalance_left();
                }
            } else {
                par -> factor--;
                if (par -> factor == -2) {
                    par = par -> rebalance_right();
                }
            }
            /* Three cases we break
             * 1) par was left-heavy and curr is its right child
             * 2) par was right-heavy and curr is its left child
             * 3) a rotation happened
             * 
             * It is safe to break because any ancestor of par should have its
             * balance factor unchanged
             */
            if (par -> factor == 0) {
                break;
            }
        }
    }

public:
    /**
     * @brief Insert all values in a range to this tree
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginnig of the range
     * @param last the end of the range
     */
    template<std::input_iterator InputIt>
    void insert(InputIt first, InputIt last) {
        for (auto it = first; it != last; it++) {
            insert(*it);
        }
    }

    /**
     * @brief Insert a single value to this tree
     * 
     * @param value the value to be copied and inserted
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted value, or the existing one if this value already exists
     * 
     * The boolean is true if insertion succeeded, namely the value is not a duplicate, false otherwise
     */
    std::pair<iterator, bool> insert(const value_type& value) {
        return insert(value_type(value));
    }

    /**
     * @brief Insert a single value to this tree
     * 
     * @param value the value to be moved and inserted
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted value, or the existing one if this value already exists
     * 
     * The boolean is true if insertion succeeded, namely the value is not a duplicate, false otherwise
     */
    std::pair<iterator, bool> insert(value_type&& value) {
        std::pair<node_type*, char> res = get_insertion_parent(key_of(value));
        if (res.second & EXISTS) {
            return std::make_pair(iterator(res.first), false);
        }
        node_type* new_node = new node_type(std::move(value));
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);

        // Populate ancestors' balance factors and rebalance
        populate_factor_after_insertion(new_node);
        element_count++;
        return std::make_pair(iterator(new_node), true);
    }

    /**
     * @brief In-place construct and insert a value to this tree
     * 
     * A value is always constructed regardless if it already exists in the tree
     * 
     * @tparam Args the type of arguments to construct the value
     * @param args the arguments to construct the value
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the emplaced value, or the existing one if this value already exists
     * 
     * The boolean is true if emplace succeeded, namely the value is not a duplicate, false otherwise
     */
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        node_type* new_node = new node_type(std::forward<Args>(args)...);
        std::pair<node_type*, char> res = get_insertion_parent(key_of(new_node -> value));
        if (res.second & EXISTS) {
            delete new_node;
            return std::make_pair(iterator(res.first), false);
        }
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);

        // Populate ancestors' balance factors and rebalance
        populate_factor_after_insertion(new_node);
        element_count++;
        return std::make_pair(iterator(new_node), true);
    }

    /**
     * @brief In-place construct and insert a value to this tree given its key and construction arguments
     * 
     * A value is constructed only if it doesn't exist in the tree
     * 
     * @tparam Args the type of arguments to construct the value
     * @param args the arguments to construct the value
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the emplaced value, or the existing one if this value already exists
     * 
     * The boolean is true if emplace succeeded, namely the value is not a duplicate, false otherwise
     */
    template<typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
        std::pair<node_type*, char> res = get_insertion_parent(key);
        if (res.second & EXISTS) {
            return std::make_pair(iterator(res.first), false);
        }
        node_type* new_node = new node_type(std::forward<Args>(args)...);
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);

        // Populate ancestors' balance factors and rebalance
        populate_factor_after_insertion(new_node);
        element_count++;
        return std::make_pair(iterator(new_node), true);
    }

    /**
     * @brief Remove the value associated with a given key
     * 
     * @param key the key of the value to erase
     * @return true if removal happened (i.e. the key was found), false otherwise
     */
    bool erase(const key_type& key) {
        iterator it = find(key);
        if (it == end()) {
            return false;
        }
        erase(it);
        return true;
    }

    /**
     * @brief Remove the value pointed by a given iterator
     * 
     * @param pos the iterator that points to the value to remove
     * @return the iterator following the removed element
     */
    iterator erase(iterator pos) {
        // Prepare return result
        iterator res = std::next(pos);
        // If the iterator doesn't have a node of underyling type avl_node
        // the user must be passing an invalid argument
        avl_node<V>* target_node = static_cast<avl_node<V>*>(pos.node);
        avl_node<V>* target_parent = target_node -> parent;
        // The lowest node that needs rebalance
        avl_node<V>* rebalance_start = target_parent;
        // Which child the node lost
        bool is_lost_left_child = target_parent -> left_child.get() == target_node;
        if (target_node -> is_leaf()) {
            /*
             *     P
             *     |    ->    P
             *     T
             */
            target_node -> orphan_self();
        } else if (!target_node -> left_child) {
            /*
             *     P           P
             *     |           |
             *     T    ->     A
             *      \
             *       A
             */
            target_parent -> link_child(std::move(target_node -> right_child), is_lost_left_child);
        } else if (!target_node -> right_child) {
            /*
             *      P          P
             *      |          |
             *      T   ->     A
             *     /
             *    A
             */
            target_parent -> link_child(std::move(target_node -> left_child), is_lost_left_child);
        } else if (!target_node -> right_child -> left_child) {
            /*
             *        P               P
             *        |(1)            |(1)
             *        T               B
             *    (2)/ \     ->   (2)/ \
             *      A   B           A  [C]
             *     / \   \         / \
             *    .   .  [C]      .   .
             */
            avl_node<V>* replacing_node = target_node -> right_child.get();
            target_parent -> link_child(std::move(target_node -> right_child), is_lost_left_child);
            replacing_node -> link_left_child(std::move(target_node -> left_child));
            // Let B take target's position
            replacing_node -> factor = target_node -> factor;
            rebalance_start = replacing_node;
            is_lost_left_child = false;
        } else {
            /*
             *          P                      P
             *          |(2)                   |(2)
             *          T                      X
             *       /(3)   \(4)            /(3)  \(4)
             *      A       C              A       C
             *     / \     / \            / \     / \
             *    .   .   B   .          .   .   B
             *           /         ->           /
             *         ...                    ...
             *         /                      /
             *        Z                      Z
             *    (1)/ \                 (1)/ \
             *      X   .                 [Y]  . 
             *       \
             *        [Y]
             */
            // Dig out the smallest node in the right subtree
            avl_node<V>* replacing_node = target_node -> right_child ->
                get_leftmost_descendant();
            rebalance_start = replacing_node -> parent;
            // (1) Let its right child take its place if it exists
            // This will make the parent forfeit the ownership to the replacing node
            if (replacing_node -> right_child) {
                replacing_node -> parent -> link_left_child(std::move(replacing_node -> right_child));
            } else {
                // Otherwise make its parent disown itself
                replacing_node -> orphan_self();
            }

            // Place the replacing node where the target node was
            target_parent -> link_child(replacing_node, is_lost_left_child); // (2)
            replacing_node -> link_left_child(std::move(target_node -> left_child)); // (3)
            replacing_node -> link_right_child(std::move(target_node -> right_child)); // (4)
            replacing_node -> factor = target_node -> factor;

            // Because replacing_node is returned from get_leftmost_descendant
            // and we know the left branch is not null,
            // the replacing node must be a left child
            is_lost_left_child = true;
        }
        delete target_node;

        for (avl_node<V>* curr = rebalance_start; curr != sentinel.get(); curr = curr -> parent) {
            bool is_curr_left_child = curr -> parent -> left_child.get() == curr;
            if (is_lost_left_child) {
                curr -> factor--;
                if (curr -> factor == -2) {
                    curr = curr -> rebalance_right();
                }
            } else {
                curr -> factor++;
                if (curr -> factor == 2) {
                    curr = curr -> rebalance_left();
                }
            }
            /**
             * We break when curr was balanced and lost a child. Its height remains unchanged
             */
            if (curr -> factor != 0) {
                break;
            }
            is_lost_left_child = is_curr_left_child;
        }
        element_count--;
        return res;
    }

    /**
     * @brief Remove the value pointed by a given iterator
     * 
     * @param pos a const iterator that points to the value to remove
     * @return the iterator following the removed element
     */
    iterator erase(const_iterator pos) {
        return erase(iterator(pos));
    }

    /**
     * @brief Remove the value pointed by a given iterator
     * 
     * @param pos a const iterator that points to the value to remove
     * @return the iterator following the removed element
     */


    /**
     * @brief Remove all values in a range
     * 
     * @param first the iterator to the first element in the range
     * @param last the iterator to one past the last element in the range
     * @return the iterator following the removed element
     */
    iterator erase(const_iterator first, const_iterator last) {
        iterator curr(first);
        while (curr != last) {
            curr = erase(curr);
        }
        return curr;
    }

    /**
     * @brief Swap content with another avl tree
     * 
     * @param other the other avl tree to swap from
     */
    void swap(avl_tree& other) noexcept {
        std::swap(key_of, other.key_of);
        std::swap(comp, other.comp);
        std::swap(sentinel, other.sentinel);
        std::swap(element_count, other.element_count);
    }

    /**
     * @brief Get the key extractor function or functor
     */
    KeyOf get_key_of() const noexcept {
        return key_of;
    }

    /**
     * @brief Get the key comparator function or functor
     */
    Comparator get_comparator() const noexcept {
        return comp;
    }

    /*
     * Testing purpose only
     */
    const avl_node<V>* get_sentinel() const noexcept {
        return sentinel.get();
    }
};

}