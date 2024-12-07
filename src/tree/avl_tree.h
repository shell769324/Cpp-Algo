#pragma once
#include <functional>
#include <concepts>
#include "binary_tree_common.h"
#include "binary_tree_base.h"
#include "binary_tree_node.h"
#include "binary_tree_iterator.h"
#include "src/common.h"
#include "src/concepts.h"
#include <optional>
#include <utility>

namespace algo {

/**
 * @brief An avl tree node. It contains height information
 * 
 * @tparam T the type of value in the node
 */
template <typename T>
class avl_node : public binary_tree_node_base<T, avl_node<T> > {
public:
    using parent_type = binary_tree_node_base<T, avl_node<T> >;

    // The number of nodes along the longest path to its leafy descendant, including itself
    // unsigned char is more than enough because avl tree is balanced
    unsigned char height;
    
    avl_node() = delete;
    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    avl_node(const avl_node& other) = delete;
    avl_node(avl_node&& other) = delete;

    /**
     * @brief Never use the destructor since it will destroy the value
     */
    ~avl_node() = delete;

    template <typename Allocator, typename... Args>
    requires std::same_as<avl_node, typename Allocator::value_type>
    static avl_node* construct(Allocator& allocator, Args&&... args) {
        avl_node* node = parent_type::construct(allocator, std::forward<Args>(args)...);
        node -> height = 1;
        return node;
    }

    template <typename Allocator>
    requires std::same_as<avl_node, typename Allocator::value_type>
    static avl_node* construct_sentinel(Allocator& allocator) {
        avl_node* node = parent_type::construct_sentinel(allocator);
        node -> height = 1;
        return node;
    }

    template <typename Allocator>
    requires std::same_as<avl_node, typename Allocator::value_type>
    avl_node* clone(Allocator& allocator) const {
        avl_node* node = parent_type::clone(allocator);
        node -> height = height;
        return node;
    }

    template <typename Allocator>
    requires std::same_as<avl_node, typename Allocator::value_type>
    void destroy(Allocator& allocator) {
        parent_type::destroy(allocator);
    }

    void fill(avl_node* other) const requires std::is_copy_constructible_v<T> {
        other -> height = height;
        other -> value = this -> value;
    }

    friend bool should_parallelize(avl_node* node1, avl_node* node2) {
        if (node1 == nullptr || node2 == nullptr) {
            return false;
        }
        unsigned char smaller = std::min(node1 -> height, node2 -> height);
        unsigned char bigger = std::max(node1 -> height, node2 -> height);
        if (smaller >= 14) {
            return true;
        }
        int work = 1 << smaller * (bigger - smaller + 1);
        return work > 10000;
    }

    /**
     * @brief Compute the difference between the height of children
     * 
     * @return the height of left child minus the height of right child
     *         If any of them doesn't exist, treat its height as zero
     */
    char compute_balance_factor() {
        char balance_factor = 0;
        if (this -> left_child) {
            balance_factor += this -> left_child -> height;
        }
        if (this -> right_child) {
            balance_factor -= this -> right_child -> height;
        }
        return balance_factor;
    }
    
    /**
     * @brief Use the heights of its children to update its own height
     */
    void update_height() {
        unsigned char new_height = 0;
        if (this -> left_child && this -> left_child -> height > new_height) {
            new_height = this -> left_child -> height;
        }
        if (this -> right_child && this -> right_child -> height > new_height) {
            new_height = this -> right_child -> height;
        }
        height = new_height + 1;
    }

    /**
     * @brief Perform a left rotation on the current node
     * 
     * The parent's height won't be changed
     * 
     * @return avl_node* the node in place of this node
     */
    avl_node* rotate_left() noexcept {
        avl_node* original_right_child = parent_type::rotate_left();
        update_height();
        original_right_child -> height = std::max(original_right_child -> height, (unsigned char) (height + 1));
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
        avl_node* original_left_child = parent_type::rotate_right();
        update_height();
        original_left_child -> height = std::max(original_left_child -> height, (unsigned char) (height + 1));
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
        if (this -> left_child -> compute_balance_factor() < 0) {
            // The height of left child will remain the same.
            // We don't need to update the height of this node
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
        if (this -> right_child -> compute_balance_factor() > 0) {
            // The height of right child will remain the same.
            // We don't need to update the height of this node
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
 * @tparam Compare key comparactor function or functor
 * @tparam Allocator the allocator that allocates and deallocates raw memory
 */
template <typename K, typename V, typename KeyOf, typename Compare = std::less<K>, typename Allocator = std::allocator<V> >
    requires binary_tree_definable<K, V, KeyOf, Compare, Allocator>
class avl_tree: public binary_tree_base<K, V, KeyOf, avl_node<V>, avl_tree<K, V, KeyOf, Compare, Allocator>, Compare, Allocator> {

public:
    using base_type = binary_tree_base<K, V, KeyOf, avl_node<V>, avl_tree<K, V, KeyOf, Compare, Allocator>, Compare, Allocator>;
    
    using key_type = base_type::key_type;
    using value_type = base_type::value_type;
    using size_type = base_type::size_type;
    using difference_type = base_type::difference_type;
    using key_compare = base_type::key_compare;
    using allocator_type = base_type::allocator_type;
    using reference = base_type::reference;
    using const_reference = base_type::const_reference;
    using pointer = base_type::pointer;
    using const_pointer = base_type::const_pointer;
    using iterator = base_type::iterator;
    using const_iterator = base_type::const_iterator;
    using reverse_iterator = base_type::reverse_iterator;
    using const_reverse_iterator = base_type::const_reverse_iterator;
    using node_type = base_type::node_type;
    using unique_ptr_type = base_type::unique_ptr_type;

    using base_type::key_of;
    using base_type::comp;
    using base_type::node_allocator;
    using base_type::sentinel;
    using base_type::element_count;
    using base_type::begin_node;
    using base_type::EXISTS;
    using base_type::IS_LEFT_CHILD;
    using base_type::get_insertion_parent;
    using base_type::__is_inorder;
    using base_type::__is_size_correct;
    using base_type::upper_bound;
    using base_type::lower_bound;
    using base_type::union_of;
    using base_type::intersection_of;
    using base_type::difference_of;
    using base_type::update_begin_node;

public:
    /**
     * @brief Construct a default empty avl tree
     */
    avl_tree() { }

    /**
     * @brief Construct an empty avl tree with a given comparator and allocator
     * 
     * @param comp the key comparator used by the tree
     * @param allocator the allocator to construct/destroy elements
     */
    explicit avl_tree(const Compare& comp, const allocator_type& allocator)
        : base_type(comp, allocator) { }

    /**
     * @brief Construct a new avl tree from range
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginning of the range
     * @param last the end of the range
     */
    template<typename InputIt>
    avl_tree(InputIt first, InputIt last) requires conjugate_uninitialized_input_iterator<InputIt, value_type>
        : avl_tree() {
        insert(first, last);
    }
    
    /**
     * @brief Construct a new avl tree from range
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginning of the range
     * @param last the end of the range
     * @param comp the key comparator used by the tree
     * @param allocator the allocator to construct/destroy elements
     */
    template<typename InputIt>
    avl_tree(InputIt first, InputIt last,
             const Compare& comp, 
             const Allocator& allocator) requires conjugate_uninitialized_input_iterator<InputIt, value_type>
        : avl_tree(comp, allocator) {
        insert(first, last);
    }

    /**
     * @brief Construct a copy of another avl tree
     * 
     * Copy constructor
     * 
     * @param other the tree to copy from
     */
    avl_tree(const avl_tree& other) = default;

    /**
     * @brief Construct a copy of another avl tree with an allocator
     * 
     * @param other the tree to copy from
     * @param allocator the allocator to construct/destroy elements
     */
    avl_tree(const avl_tree& other, const Allocator& allocator) 
        : base_type(other, allocator) { }

    /**
     * @brief Construct a copy of another avl tree by move
     * 
     * Move constructor
     * 
     * @param other the tree to move from
     */
    avl_tree(avl_tree&& other) = default;

    /**
     * @brief Construct a copy of another avl tree with an allocator by move
     * 
     * @param other the tree to move from
     * @param allocator the allocator to construct/destroy eleents
     */
    avl_tree(avl_tree&& other, const Allocator& allocator) 
        : base_type(std::move(other), allocator) { }

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

private:
    /**
     * @brief Populate the height change after this node is inserted
     * 
     * All ancestors of this node will have their heights updated
     * 
     * Rebalance is performed if necessary
     * 
     * @param new_node the newly inserted node
     */
    static void adjust_after_insertion(node_type* new_node, node_type* end) {
        for (node_type* curr = new_node, *par = new_node -> parent; par != end;
                curr = par, par = par -> parent) {
            par -> height = std::max(par -> height, (unsigned char) (curr -> height + 1));
            char balance_factor = par -> compute_balance_factor();
            switch (balance_factor) {
            // Height change needs to percolate up more
            case -1:
            case 1:
                break;
            // Rebalance will absorb height change
            // Fall through and return
            case 2:
                par -> rebalance_left();
                return;
            case -2:
                par -> rebalance_right();
                return;
            case 0:
                return;
            }
        }
    }

    void adjust_after_insertion(node_type* new_node) {
        adjust_after_insertion(new_node, sentinel);
    }

public:
    /**
     * @brief Insert all values in a range to this tree
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginnig of the range
     * @param last the end of the range
     */
    template<typename InputIt>
    void insert(InputIt first, InputIt last) requires conjugate_uninitialized_input_iterator<InputIt, value_type> {
        for (InputIt& it = first; it != last; ++it) {
            insert(*it);
        }
    }

private:
    template<typename... Args>
    std::pair<iterator, bool> insert_helper(const key_type& key, Args&&... args) {
        std::pair<node_type*, char> res = get_insertion_parent(key);
        if (res.second & EXISTS) {
            return std::make_pair(iterator(res.first), false);
        }
        node_type* new_node = node_type::construct(node_allocator, std::forward<Args>(args)...);
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);
        // Populate ancestors' heights and rebalance
        adjust_after_insertion(new_node);
        update_begin_node(new_node);
        ++element_count;
        return std::make_pair(iterator(new_node), true);
    }

public:
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
    std::pair<iterator, bool> insert(const value_type& value) requires std::is_copy_constructible_v<value_type> {
        return insert_helper(key_of(value), value);
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
    std::pair<iterator, bool> insert(value_type&& value) requires std::move_constructible<value_type> {
        return insert_helper(key_of(value), std::move(value));
    }

public:
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
        node_type* new_node = node_type::construct(node_allocator, std::forward<Args>(args)...);
        std::pair<node_type*, char> res = get_insertion_parent(key_of(new_node -> value));
        if (res.second & EXISTS) {
            new_node -> destroy(node_allocator);
            return std::make_pair(iterator(res.first), false);
        }
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);

        // Populate ancestors' heights and rebalance
        adjust_after_insertion(new_node);
        update_begin_node(new_node);
        ++element_count;
        return std::make_pair(iterator(new_node), true);
    }

    /**
     * @brief In-place construct and insert a value to this tree
     * 
     * A value is always constructed regardless if it already exists in the tree
     * 
     * @tparam Args the type of arguments to construct the value
     * @param hint an iterator that is close to the insertion location of the new value. If the iterator
     *             is far from insertion location, the value is emplaced as if emplace(std::forward<Args>(args)...)
     * @param args the arguments to construct the value
     * @return an iterator to the inserted element, or to the element that prevented the insertion
     */
    template<typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
        node_type* new_node = node_type::construct(node_allocator, std::forward<Args>(args)...);
        std::pair<node_type*, char> res = get_insertion_parent(hint, key_of(new_node -> value));
        if (res.second & EXISTS) {
            new_node -> destroy(node_allocator);
            return iterator(res.first);
        }
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);

        // Populate ancestors' heights and rebalance
        adjust_after_insertion(new_node);
        update_begin_node(new_node);
        ++element_count;
        return iterator(new_node);
    }

    /**
     * @brief In-place construct and insert a value to this tree given its key and construction arguments
     * 
     * A value is constructed only if it doesn't exist in the tree
     * 
     * @tparam Args the type of arguments to construct the value
     * @param key the key associated with the new value to insert
     * @param args the arguments to construct the value
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the emplaced value, or the existing one if this value already exists
     * 
     * The boolean is true if emplace succeeded, namely the value is not a duplicate, false otherwise
     */
    template<typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
        return insert_helper(key, std::forward<Args>(args)...);
    }

private:
    /**
     * @brief Remove the node pointed by an iterator
     * 
     * The node is not deallocated
     * 
     * @param pos the iterator to the node to remove
     * @return iterator following the removed node
     */
    static iterator extract(iterator pos, node_type* end) {
        // Prepare return result
        iterator res = std::next(pos);
        // If the iterator doesn't have a node of underyling type avl_node
        // the user must be passing an invalid argument
        node_type* target_node = static_cast<node_type*>(pos.node);
        node_type* target_parent = target_node -> parent;
        // The lowest node that needs rebalance
        node_type* rebalance_start = target_parent;
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
            replacing_node -> height = target_node -> height;
            rebalance_start = replacing_node;
        } else {
            /*
             *          P                      P
             *          |(2)                   |(2)
             *          T                      X
             *       /(3)  \(4)             /(3)  \(4)
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
            replacing_node -> height = target_node -> height;
        }

        for (node_type* curr = rebalance_start; curr != end; curr = curr -> parent) {
            curr -> update_height();
            char balance_factor = curr -> compute_balance_factor();
            switch (balance_factor) {
            case -1:
            case 1:
                goto loop_end;
            case -2:
                curr = curr -> rebalance_right();
                break;
            case 2:
                curr = curr -> rebalance_left();
                break;
            // Do nothing. Height change needs to percolate up more
            case 0:
                break;
            }
        }
        loop_end:
        return res;
    }

public:
    /**
     * @brief Remove the value associated with a given key
     * 
     * @param key the key of the value to erase
     * @return true if removal happened (i.e. the key was found), false otherwise
     */
    bool erase(const key_type& key) {
        iterator it = this -> find(key);
        if (it == this -> end()) {
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
        bool was_begin_node = pos.node == begin_node;
        iterator res = extract(pos, sentinel);
        if (was_begin_node) {
            begin_node = res.node;
        }
        (static_cast<node_type*>(pos.node)) -> destroy(node_allocator);
        --element_count;
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
    void swap(avl_tree& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        static_cast<base_type&>(*this).swap(static_cast<base_type&>(other));
    }

    /**
     * @brief Check equality of two avl trees
     * 
     * @param tree1 the first avl tree
     * @param tree2 the second avl tree
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const avl_tree& tree1, const avl_tree& tree2) requires equality_comparable<value_type> {
        return container_equals(tree1, tree2);
    }

    /**
     * @brief Compare two avl trees
     * 
     * @param tree1 the first avl tree
     * @param tree2 the second avl tree
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const avl_tree& tree1, const avl_tree& tree2) requires less_comparable<value_type> {
        return container_three_way_comparison(tree1, tree2);
    }

    /**
     * @brief Get the key extractor function or functor
     */
    KeyOf get_key_of() const noexcept {
        return key_of;
    }

    /**
     * @brief Get the key compare function or functor
     */
    Compare key_comp() const noexcept {
        return comp;
    }

    /*
     * Testing purpose only
     */
    const avl_node<V>* __get_sentinel() const noexcept {
        return sentinel;
    }

    base_type::node_allocator_type& __get_node_allocator() noexcept {
        return node_allocator;
    }

private:
    /**
     * @brief Join a tree into the right spine of another tree
     * 
     * All elements in the destination must be less than all elements
     * in the source
     * 
     * @param dest a unique ptr to the tree to join into. Must be nonnull
     * @param src a unique ptr to the tree that will join the destination tree.
     *            Can be null
     * @return the result of the join
     */
    static unique_ptr_type join_right(unique_ptr_type dest, unique_ptr_type src) {
        if (!src) {
            return dest;
        }
        if (!dest -> right_child) {
            // Note that both dest and src are valid avl tree root
            // src -> height <= dest -> height <= 2
            // So the linking here will preserve the avl constraint
            dest -> link_right_child(std::move(src));
            dest -> update_height();
            return dest;
        }
        node_type* middle_node = dest -> get_rightmost_descendant();
        // dest may no longer be the root after rebalancing in extract
        // Releasing it to avoid having two unique pointers owning the same raw pointer
        node_type* raw_dest = dest.release();
        extract(middle_node, nullptr);
        node_type* new_dest = raw_dest -> parent ? raw_dest -> parent : raw_dest;
        return join_right(unique_ptr_type(new_dest), std::move(src), unique_ptr_type(middle_node));
    }

    /**
     * @brief Join a tree into the right spine of another tree along with a middle node
     * 
     * All elements in the destination must be less than the middle node
     * All elements in the source must be greater than the middle node
     * 
     * @param dest a unique ptr to the tree to join into. Must be nonnull
     * @param src a unique ptr to the tree that will join the destination tree.
     *            Can be null
     * @param middle_node a unique ptr to the middle node. Must be nonnull
     * @return the result of the join
     */
    static unique_ptr_type join_right(unique_ptr_type dest, unique_ptr_type src, unique_ptr_type middle_node) {
        unsigned char max_balancing_height = src ? src -> height + 1 : 1;
        node_type* curr = dest.get();
        node_type* parent = nullptr;
        while (curr && curr -> height > max_balancing_height) {
            parent = curr;
            curr = curr -> right_child.get();
        }

        node_type* middle_ptr = middle_node.release();
        node_type* raw_dest = dest.release();
        if (parent) {
            parent -> link_right_child(middle_ptr);
        }
        
        middle_ptr -> safe_link_left_child(curr);
        middle_ptr -> safe_link_right_child(std::move(src));
        middle_ptr -> update_height();
        adjust_after_insertion(middle_ptr, nullptr);
        return unique_ptr_type(raw_dest -> parent ? raw_dest -> parent : raw_dest);
    }

    /**
     * @brief Join a tree into the left spine of another tree
     * 
     * All elements in the destination must be greater than all elements
     * in the source
     * 
     * @param dest a unique ptr to the tree to join into. Must be nonnull
     * @param src a unique ptr to the tree that will join the destination tree.
     *            Can be null
     * @return the result of the join
     */
    static unique_ptr_type join_left(unique_ptr_type dest, unique_ptr_type src) {
        if (!src) {
            return dest;
        }
        if (!dest -> left_child) {
            // Note that both dest and src are valid avl tree root
            // src -> height <= dest -> height <= 2
            // So the linking here will preserve the avl constraint
            dest -> link_left_child(std::move(src));
            dest -> update_height();
            return dest;
        }
        node_type* middle_node = dest -> get_leftmost_descendant();
        // dest may no longer be the root after rebalancing in extract
        // Releasing it to avoid having two unique pointers pointint owning the same raw pointer
        node_type* raw_dest = dest.release();
        extract(middle_node, nullptr);
        node_type* new_dest = raw_dest -> parent ? raw_dest -> parent : raw_dest;
        return join_left(unique_ptr_type(new_dest), std::move(src), unique_ptr_type(middle_node));
    }

    /**
     * @brief Join a tree into the left spine of another tree along with a middle node
     * 
     * All elements in the destination must be greater than the middle node
     * All elements in the source must be less than the middle node
     * 
     * @param dest a unique ptr to the tree to join into. Must be nonnull
     * @param src a unique ptr to the tree that will join the destination tree.
     *            Can be null
     * @param middle_node a unique ptr to the middle node. Must be nonnull
     * @return the result of the join
     */
    static unique_ptr_type join_left(unique_ptr_type dest, unique_ptr_type src, unique_ptr_type middle_node) {
        unsigned char max_balancing_height = src ? src -> height + 1 : 1;
        node_type* curr = dest.get();
        node_type* parent = nullptr;
        while (curr && curr -> height > max_balancing_height) {
            parent = curr;
            curr = curr -> left_child.get();
        }

        node_type* middle_ptr = middle_node.release();
        node_type* raw_dest = dest.release();
        if (parent) {
            parent -> link_left_child(middle_ptr);
        }
        middle_ptr -> safe_link_left_child(std::move(src));
        middle_ptr -> safe_link_right_child(curr);
        middle_ptr -> update_height();
        
        adjust_after_insertion(middle_ptr, nullptr);
        return unique_ptr_type(raw_dest -> parent ? raw_dest -> parent : raw_dest);
    }

public:
    /**
     * @brief Join two trees into a node
     * 
     * Precondition: max(left) < middle -> value < min(right)
     * 
     * @param left the tree with smaller values
     * @param right the tree with greater values
     * @param middle a leafy node without parent that has a value between the left and right true
     * @return node_type* the joined node
     */
    static unique_ptr_type join(unique_ptr_type left, unique_ptr_type middle, unique_ptr_type right) {
        if (!left && !right) {
            middle -> height = 1;
            return middle;
        }

        if (left && (!right || left -> height >= right -> height)) {
            return join_right(std::move(left), std::move(right), std::move(middle));
        }
        return join_left(std::move(right), std::move(left), std::move(middle));
    }

    static unique_ptr_type join(unique_ptr_type left, unique_ptr_type right) {
        if (!left && !right) {
            return nullptr;
        }
        if (left && (!right || left -> height >= right -> height)) {
            return join_right(std::move(left), std::move(right));
        }
        return join_left(std::move(right), std::move(left));
    }

private:
    /**
     * @brief A helper for splitting a tree by a key of a given node
     */
    template<typename Resolver>
    std::pair<unique_ptr_type, bool> split_helper(unique_ptr_type root, unique_ptr_type divider, const key_type& divider_key, Resolver& resolver) {
        if (!root) {
            return std::make_pair(std::move(divider), false);
        }
        const key_type& root_key = key_of(root -> value);
        int key_comp_res = this -> key_comp_wrapper(divider_key, root_key);
        if (key_comp_res == 0) {
            if (resolver(root -> value, divider -> value)) {
                if (root -> left_child) {
                    root -> left_child -> parent = nullptr;
                }
                if (root -> right_child) {
                    root -> right_child -> parent = nullptr;
                }
                divider.release() -> destroy(node_allocator);
                return std::make_pair(std::move(root), true);
            }
            divider -> left_child.reset(root -> orphan_left_child());
            divider -> right_child.reset(root -> orphan_right_child());
            root.release() -> destroy(node_allocator);
            return std::make_pair(std::move(divider), true);
        }
        root -> height = 1;
        unique_ptr_type root_left_child(root -> orphan_left_child());
        unique_ptr_type root_right_child(root -> orphan_right_child());
        std::pair<unique_ptr_type, bool> split_result;
        if (key_comp_res < 0) {
            split_result = split_helper(std::move(root_left_child), std::move(divider), divider_key, resolver);
            split_result.first -> right_child = join(std::move(split_result.first -> right_child), std::move(root), std::move(root_right_child));
        } else {
            split_result = split_helper(std::move(root_right_child), std::move(divider), divider_key, resolver);
            split_result.first -> left_child = join(std::move(root_left_child), std::move(root), std::move(split_result.first -> left_child));
        }
        return split_result;
    }

public:
    /**
     * @brief Split a tree by a key of a given node
     * 
     * If the key exists in the tree, the resolver will determine which value to remove (and deallocate)
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the tree
     *         It inputs two values and output true if the first value should be picked
     * @param root the avl tree to split.
     * @param divider the node that contains the key.
     * @param resolver a conflict resolution function
     * @return a pair of node and boolean such that the node's left child and right child are complementary
     *         partitions of the input tree. Note that the returning node is no way a balance tree. It is just a way to return multiple things.
     *         without having to resort to tuple, which is slow. The boolean is true if a collision occurs. False otherwise.
     */
    template<typename Resolver=chooser<value_type> >
    std::pair<unique_ptr_type, bool> split(unique_ptr_type root, unique_ptr_type divider, Resolver resolver = Resolver()) 
        requires is_resolver<value_type, Resolver> {
        return split_helper(std::move(root), std::move(divider), key_of(divider -> value), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree union_of_helper(avl_tree tree1, avl_tree tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor, Resolver resolver) 
        requires is_resolver<value_type, Resolver> {
        if (tree1.empty()) {
            return tree2;
        }
        if (tree2.empty()) {
            return tree1;
        }
        
        node_type* root1 = tree1.sentinel -> orphan_left_child();
        tree1.element_count = 0;
        node_type* root2 = tree2.sentinel -> orphan_left_child();
        tree2.element_count = 0;
        unique_ptr_type res;
        if (executor.has_value()) {
            res = tree1.union_of(unique_ptr_type(root1), unique_ptr_type(root2), executor.value().get(), resolver);
        } else {
            res = tree1.union_of(unique_ptr_type(root1), unique_ptr_type(root2), resolver);
        }
        tree1.sentinel -> link_left_child(std::move(res));
        tree1.begin_node = tree1.sentinel -> get_leftmost_descendant();
        tree1.element_count = std::distance(tree1.cbegin(), tree1.cend());
        return tree1;
    }

    /**
     * @brief Compute the union of two avl trees
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the tree
     *         It inputs two values and output true if the first value should be picked
     * @param tree1 the first operand of the union operation
     * @param tree2 the second operand of the union operation
     * @param resolver a conflict resolution function
     * @return the union of the trees 
     */
    template<typename Resolver=chooser<value_type> >
    friend avl_tree union_of(avl_tree tree1, avl_tree tree2, Resolver resolver=Resolver()) 
        requires is_resolver<value_type, Resolver> {
        return union_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>>(), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree union_of(avl_tree tree1, avl_tree tree2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) requires is_resolver<value_type, Resolver> {
        return union_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>> (std::ref(executor)), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree intersection_of_helper(avl_tree tree1, avl_tree tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor, Resolver resolver) 
        requires is_resolver<value_type, Resolver> {
        if (tree1.empty()) {
            return tree1;
        }
        if (tree2.empty()) {
            return tree2;
        }
        
        node_type* root1 = tree1.sentinel -> orphan_left_child();
        tree1.element_count = 0;
        node_type* root2 = tree2.sentinel -> orphan_left_child();
        tree2.element_count = 0;
        unique_ptr_type res;

        if (executor.has_value()) {
            res = tree1.intersection_of(unique_ptr_type(root1), unique_ptr_type(root2), executor.value().get(), resolver);
        } else {
            res = tree1.intersection_of(unique_ptr_type(root1), unique_ptr_type(root2), resolver);
        }
        tree1.sentinel -> safe_link_left_child(std::move(res));
        tree1.begin_node = tree1.sentinel -> get_leftmost_descendant();
        tree1.element_count = std::distance(tree1.cbegin(), tree1.cend());
        return tree1;
    }

    /**
     * @brief Compute the intersection of two avl trees
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the tree
     *         It inputs two values and output true if the first value should be picked
     * @param tree1 the first operand of the intersection operation
     * @param tree2 the second operand of the intersection operation
     * @param resolver a conflict resolution function
     * @return the intersection of the trees
     */
    template<typename Resolver=chooser<value_type> >
    friend avl_tree intersection_of(avl_tree tree1, avl_tree tree2, Resolver resolver=Resolver()) 
        requires is_resolver<value_type, Resolver> {
        return intersection_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>>(), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree intersection_of(avl_tree tree1, avl_tree tree2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) requires is_resolver<value_type, Resolver> {
        return intersection_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>> (std::ref(executor)), resolver);
    }

    friend avl_tree difference_of_helper(avl_tree tree1, avl_tree tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor) {
        if (tree1.empty() || tree2.empty()) {
            return tree1;
        }
        
        node_type* root1 = tree1.sentinel -> orphan_left_child();
        tree1.element_count = 0;
        node_type* root2 = tree2.sentinel -> orphan_left_child();
        tree2.element_count = 0;
        unique_ptr_type res;

        if (executor.has_value()) {
            res = tree1.difference_of(unique_ptr_type(root1), unique_ptr_type(root2), executor.value().get());
        } else {
            res = tree1.difference_of(unique_ptr_type(root1), unique_ptr_type(root2));
        }
        tree1.sentinel -> safe_link_left_child(std::move(res));
        tree1.begin_node = tree1.sentinel -> get_leftmost_descendant();
        tree1.element_count = std::distance(tree1.cbegin(), tree1.cend());
        return tree1;
    }

    /**
     * @brief Compute the difference of two avl trees
     * 
     * @param tree1 the tree to subtract from
     * @param tree2 the tree that subtracts
     * @return the difference of the trees
     */
    friend avl_tree difference_of(avl_tree tree1, avl_tree tree2) {
        return difference_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>>());
    }

    friend avl_tree difference_of(avl_tree tree1, avl_tree tree2, thread_pool_executor& executor) {
        return difference_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>> (std::ref(executor)));
    }

private:
    constexpr static unsigned ERROR = 0xffffffff;

    unsigned __is_height_correct_helper(node_type* node) const noexcept {
        if (!node) {
            return 0;
        }
        unsigned actual_left_height = __is_height_correct_helper(node -> left_child.get());
        if (actual_left_height == ERROR) {
            return ERROR;
        }
        unsigned actual_right_height = __is_height_correct_helper(node -> right_child.get());
        if (actual_right_height == ERROR) {
            return ERROR;
        }
        unsigned actual_height = 1 + std::max(actual_left_height, actual_right_height);
        if (actual_height != node -> height) {
            return ERROR;
        }
        // If balance factor is more than 1, tree is not balanced
        if (std::max(actual_left_height, actual_right_height) -
            std::min(actual_left_height, actual_right_height) > 1) {
            return ERROR;
        }
        return actual_height;
    }

    bool __is_height_correct(node_type* node) const noexcept {
        return __is_height_correct_helper(node) != ERROR;
    }

public:
    bool __is_valid() const noexcept {
        node_type* smallest = sentinel -> get_leftmost_descendant();
        return __is_height_correct(sentinel -> left_child.get()) 
            & sentinel -> __is_parent_child_link_mutual() 
            & __is_inorder(const_iterator(smallest), this -> cend()) 
            & __is_size_correct(sentinel -> left_child.get(), element_count)
            & this -> __is_begin_node_correct();
    }
};
}
