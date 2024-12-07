#pragma once
#include <functional>
#include <concepts>
#include "binary_tree_common.h"
#include "binary_tree_base.h"
#include "binary_tree_node.h"
#include "binary_tree_iterator.h"
#include "src/common.h"
#include "src/concepts.h"


namespace algo {

/**
 * @brief A red black tree node. It has a binary coloring scheme
 * 
 * @tparam T the type of value in the node
 */
template <typename T>
class red_black_node : public binary_tree_node_base<T, red_black_node<T> > {
public:

    using parent_type = binary_tree_node_base<T, red_black_node<T> >;

    // first bit is 1 only if this is a red node
    // The rest of the bits is the black height of the node
    unsigned char color_data;
    constexpr static unsigned char RED_MASK = 0b10000000;
    constexpr static unsigned char RED_LEAF_CODE = 0b10000000;

    /**
     * @brief Create a new red black node
     */
    red_black_node() = delete;

    // There should never be a scenario where we need to duplicate a node
    // We should always operate on pointers to node
    red_black_node(const red_black_node& other) = delete;
    red_black_node(red_black_node&& other) = delete;

    /**
     * @brief Destroy the red black node object
     */
    ~red_black_node() = delete;

    template <typename Allocator, typename... Args>
    requires std::same_as<red_black_node, typename Allocator::value_type>
    static red_black_node* construct(Allocator& allocator, Args&&... args) {
        red_black_node* node = parent_type::construct(allocator, std::forward<Args>(args)...);
        node -> color_data = RED_LEAF_CODE;
        return node;
    }

    template <typename Allocator>
    requires std::same_as<red_black_node, typename Allocator::value_type>
    static red_black_node* construct_sentinel(Allocator& allocator) {
        red_black_node* node = parent_type::construct_sentinel(allocator);
        node -> color_data = RED_LEAF_CODE;
        return node;
    }

    /**
     * @brief Create a shallow copy of this node
     * 
     * Parent or child pointers are not moved
     * 
     * @return red_black_node* a shallow copy of this node
     */
    template <typename Allocator>
    requires std::same_as<red_black_node, typename Allocator::value_type>
    red_black_node* clone(Allocator& allocator) const {
        red_black_node* node = parent_type::clone(allocator);
        node -> color_data = color_data;
        return node;
    }

    friend bool should_parallelize(red_black_node* node1, red_black_node* node2) {
        if (!node1 || !node2) {
            return false;
        }
        unsigned char height1 = node1 -> get_black_height();
        unsigned char height2 = node2 -> get_black_height();
        unsigned char smaller = std::min(height1, height2);
        unsigned char bigger = std::max(height1, height2);
        if (smaller >= 8) {
            return true;
        }
        int work = 1 << smaller * (bigger - smaller + 1);
        return work > 256;
    }

    /**
     * @brief Check is a node is red
     */
    static bool is_red(const red_black_node* node) noexcept {
        return node && (node -> color_data & RED_MASK);
    }

    /**
     * @brief Check is a node is black
     * 
     * A null node is also considered as black
     */
    static bool is_black(const red_black_node* node) noexcept {
        return !node || !(node -> color_data & RED_MASK);
    }

    /**
     * @brief Mark a node as red
     */
    void mark_as_red() noexcept {
        color_data = color_data | RED_MASK;
    }

    /**
     * @brief Mark a node as black
     */
    void mark_as_black() noexcept {
        color_data = color_data & (~RED_MASK);
    }

    /**
     * @brief Mark this node as the same color as the other node
     */
    void mark_as_same_color(red_black_node* other) noexcept {
        if (is_red(other)) {
            mark_as_red();
        } else {
            mark_as_black();
        }
    }

    /**
     * @brief Get the number of black nodes from this node to any of its leafy descendant
     */
    unsigned char get_black_height() const noexcept {
        return color_data & (~RED_MASK);
    }

    /**
     * @brief Set black height field
     */
    void set_black_height(unsigned char new_height) noexcept {
        color_data = (color_data & RED_MASK) | new_height;
    }

    /**
     * @brief Increment black height field
     */
    void increment_height(char delta) noexcept {
        set_black_height(get_black_height() + delta);
    }

    /**
     * @brief Populate the black height field from its children
     */
    void update_black_height() noexcept {
        unsigned char new_height = is_black(this) ? 1 : 0;
        if (this -> left_child) {
            new_height += this -> left_child -> get_black_height();
        } else if (this -> right_child) {
            new_height += this -> right_child -> get_black_height();
        }
        set_black_height(new_height);
    }

    /**
     * @brief Swap two nodes by swapping their parents and children
     * 
     * @param other the node to swap with
     */
    void deep_swap(red_black_node* other) noexcept {
        if (this == other) {
            return;
        }
        if (other -> parent == this) {
            other -> deep_swap(this);
            return;
        }
        std::swap(color_data, other -> color_data);
        if (this -> parent == other) {
            bool is_this_left_child = this -> is_left_child();
            bool is_other_left_child = other -> parent ? other -> is_left_child() : true;
            red_black_node* other_parent = other -> parent ? other -> orphan_self() : nullptr;
            if (is_this_left_child) {
                other -> template orphan_left_child<false>();
            } else {
                other -> template orphan_right_child<false>();
            }
            red_black_node* sibling = is_this_left_child ? other -> orphan_right_child() : other -> orphan_left_child();
            other -> safe_link_left_child(this -> orphan_left_child());
            other -> safe_link_right_child(this -> orphan_right_child());

            if (other_parent) {
                other_parent -> link_child(this, is_other_left_child);
            }  else {
                this -> parent = nullptr;
            }
            this -> safe_link_child(sibling, !is_this_left_child);
            this -> link_child(other, is_this_left_child);
            return;
        }

        bool is_other_left_child = other -> parent ? other -> is_left_child() : true;
        red_black_node* other_parent = other -> parent ? other -> orphan_self() : nullptr;
        red_black_node* other_left = other -> orphan_left_child();
        red_black_node* other_right = other -> orphan_right_child();
        if (this -> parent) {
            this -> parent -> link_child(other, this -> is_left_child());
        } else {
            other -> parent = nullptr;
        }
        other -> safe_link_left_child(this -> orphan_left_child());
        other -> safe_link_right_child(this -> orphan_right_child());

        if (other_parent) {
            other_parent -> link_child(this, is_other_left_child);
        }  else {
            this -> parent = nullptr;
        }
        this -> safe_link_left_child(other_left);
        this -> safe_link_right_child(other_right);
    }
};


/**
 * @brief A red black tree implementation
 * 
 * @tparam K the type of the key
 * @tparam V the type of the value
 * @tparam KeyOf key extractor function or functor
 * @tparam Compare key comparactor function or functor
 */
template <typename K, typename V, typename KeyOf, typename Compare = std::less<K>, typename Allocator = std::allocator<V> >
    requires binary_tree_definable<K, V, KeyOf, Compare, Allocator>
class red_black_tree: public binary_tree_base<K, V, KeyOf, red_black_node<V>, red_black_tree<K, V, KeyOf, Compare, Allocator>, Compare, Allocator> {

public:
    using base_type = binary_tree_base<K, V, KeyOf, red_black_node<V>, red_black_tree<K, V, KeyOf, Compare, Allocator>, Compare, Allocator>;

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

private:
    constexpr static const unsigned int SWAP_CONTENT_THRESHOLD = sizeof(void*) * 10;

public:
    constexpr static const unsigned char HAS_CONFLICT = 0x7e;

public:
    /**
     * @brief Construct a default empty red black tree
     */
    red_black_tree() { }

    /**
     * @brief Construct an empty red black tree with a comparator and an allocator
     * 
     * @param comp the key comparator used by the tree
     * @param allocator the allocator to construct/destroy elements
     */
    explicit red_black_tree(const Compare& comp, const allocator_type& allocator) 
        : base_type(comp, allocator) { }

    /**
     * @brief Construct a new red black tree from range
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginning of the range
     * @param last the end of the range
     */
    template<typename InputIt>
    red_black_tree(InputIt first, InputIt last) requires conjugate_uninitialized_input_iterator<InputIt, value_type> 
        : red_black_tree() {
        insert(first, last);
    }

    /**
     * @brief Construct a new red black tree from range
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginning of the range
     * @param last the end of the range
     * @param comp the key comparator used by the tree
     * @param allocator the allocator to construct/destroy elements
     */
    template<typename InputIt>
    red_black_tree(InputIt first, InputIt last,
             const Compare& comp, 
             const Allocator& allocator) requires conjugate_uninitialized_input_iterator<InputIt, value_type>
        : red_black_tree(comp, allocator) {
        insert(first, last);
    }

    /**
     * @brief Construct a copy of another red black tree
     * 
     * Copy constructor
     * 
     * @param other the tree to copy from
     */
    red_black_tree(const red_black_tree& other) = default;

    /**
     * @brief Construct a copy of another red black tree with an allocator
     * 
     * @param other the tree to copy from
     * @param allocator the allocator to construct/destroy elements
     */
    red_black_tree(const red_black_tree& other, const Allocator& allocator) 
        : base_type(other, allocator) { }

    /**
     * @brief Construct a copy of another red black tree
     * 
     * Move constructor
     * 
     * @param other the tree to move from
     */
    red_black_tree(red_black_tree&& other) = default;

    /**
     * @brief Construct a copy of another red black tree with an allocator by move
     * 
     * @param other the tree to move from
     * @param allocator the allocator to construct/destroy eleents
     */
    red_black_tree(red_black_tree&& other, const Allocator& allocator) 
        : base_type(std::move(other), allocator) { }

    /**
     * @brief Copy assignment operator
     * 
     * @param other the tree to copy from
     * @return a reference to this tree
     */
    red_black_tree& operator=(const red_black_tree& other) {
        if (this == &other) {
            return *this;
        }
        red_black_tree tmp(other);
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
    red_black_tree& operator=(red_black_tree&& other) noexcept {
        swap(other);
        return *this;
    }

private:
    /**
     * @brief Given a newly added red node that has a red parent,
     *        restore the "no double red" constraint
     * 
     * @param new_node the newly added node
     * @param end the exclusive end of the path from the new node to the root of the tree 
     */
    static void fix_double_red(node_type* new_node, node_type* end) {
        // RP stands for red parent. RU stands for red uncle. RG stands for red grandparent.
        // BP stands for black parent. BU stands for black uncle. BG stands for black grandparent
        // C stands for current node that is always red.
        // RC stands for red current node. BC stands for black current node.
        for (node_type* curr = new_node; curr -> parent != end;) {
            node_type* parent = curr -> parent;
            /**
             * Parent is black. Red red conflict is solved
             * 
             *      BP
             *      |
             *      C
             * 
             */
            if (node_type::is_black(parent)) {
                return;
            }
            node_type* grandparent = parent -> parent;
            /**
             * Parent is the root. Parent can turn black without causing red red conflict
             * 
             *   End node      End node
             *      |            |
             *      RP    ->     BP
             *      |            |
             *      C            C
             * 
             */
            if (grandparent == end) {
                parent -> mark_as_black();
                parent -> increment_height(1);
                return;
            }
            bool is_curr_left_child = curr -> is_left_child();
            bool is_parent_left_child = parent -> is_left_child();
            node_type* uncle = is_parent_left_child ? grandparent -> right_child.get() : grandparent -> left_child.get();
            /**
             * Both parent and uncle are red. We can make them black and grandparent red
             * Now grandparent may have red red conflict
             *        BG               RG
             *       /  \             /  \  
             *      RP  RU    ->     BP  BU 
             *      |                |
             *      C                C
             * 
             */
            if (node_type::is_red(uncle)) {
                parent -> mark_as_black();
                parent -> increment_height(1);
                uncle -> mark_as_black();
                uncle -> increment_height(1);
                grandparent -> mark_as_red();
                curr = grandparent;
            } else {
                grandparent -> mark_as_red();
                grandparent -> increment_height(-1);
                if (is_parent_left_child) {
                    if (is_curr_left_child) {
                        /**
                         * Red parent and black uncle. Perform a right rotation on 
                         * grandparent for parent to take its place
                         *        BG               RG              BP
                         *       /  \             /  \            /  \
                         *      RP  BU    ->     BP  BU    ->    C    RG
                         *     /                /                      \
                         *    C                C                       BU
                         * 
                         */
                        parent -> increment_height(1);
                        parent -> mark_as_black();
                    } else {
                        /**
                         * Red parent and black uncle. Current node is a right child
                         * Perform double rotation to make RC the new grandparent
                         *        BG               RG              BC
                         *       /  \             /  \            /  \
                         *      RP  BU    ->     BC  BU    ->    RP  RG
                         *        \             /                      \
                         *        RC           RP                      BU
                         * 
                         */
                        curr -> increment_height(1);
                        curr -> mark_as_black();
                        parent -> rotate_left();
                    }
                    grandparent -> rotate_right();
                } else {
                    if (is_curr_left_child) {
                        /**
                         * Red parent and black uncle. Current node is a left child
                         * Perform double rotation to make RC the new grandparent
                         *        BG               RG              BC
                         *       /  \             /  \            /  \
                         *      BU  RP    ->     BU  BC    ->    RG   RP
                         *         /                   \        /
                         *        RC                    RP     BU
                         * 
                         */
                        curr -> increment_height(1);
                        curr -> mark_as_black();
                        parent -> rotate_right();
                    } else {
                        /**
                         * Red parent and black uncle. Perform a right rotation on 
                         * grandparent for parent to take its place
                         *        BG               RG              BP
                         *       /  \             /  \            /  \
                         *      BU  RP    ->     BU  BP    ->    RG   C
                         *            \                \        /
                         *             C                C      BU
                         * 
                         */
                        parent -> increment_height(1);
                        parent -> mark_as_black();
                    }
                    grandparent -> rotate_left();
                }
                return;
            }
        }
    }

    void fix_double_red(node_type* new_node) {
        return fix_double_red(new_node, sentinel);
    }

    template<typename... Args>
    std::pair<iterator, bool> insert_helper(const key_type& key, Args&&... args) {
        std::pair<node_type*, char> res = get_insertion_parent(key);
        if (res.second & EXISTS) {
            return std::make_pair(iterator(res.first), false);
        }
        node_type* new_node = node_type::construct(node_allocator, std::forward<Args>(args)...);
        res.first -> link_child(new_node, res.second & IS_LEFT_CHILD);
        ++element_count;
        fix_double_red(new_node);
        update_begin_node(new_node);
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
        fix_double_red(new_node);
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
        fix_double_red(new_node);
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

    /**
     * @brief Insert all values in a range to this tree
     * 
     * @tparam InputIt the type of the iterator
     * @param first the beginnig of the range
     * @param last the end of the range
     */
    template<typename InputIt>
    void insert(InputIt first, InputIt last) requires conjugate_uninitialized_input_iterator<InputIt, value_type> {
        for (InputIt it = first; it != last; ++it) {
            insert(*it);
        }
    }

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

    static iterator extract(iterator pos, node_type* end) {
        // Prepare return result
        iterator res = std::next(pos);
        // If the iterator doesn't have a node of underyling type red black_node
        // the user must be passing an invalid argument
        node_type* target_node = static_cast<node_type*>(pos.node);
        // First reduce the case of a node with two children to 
        // the cases of a node with at most one child
        // Find the greatest node less than the target node and swap their value
        // That node has only one child and is easier to deal with
        if (target_node -> left_child && target_node -> right_child) {
            node_type* replacing_node = target_node -> left_child -> get_rightmost_descendant();
            // If the value type is move assignable and the size is smaller than certain threshold
            // We move the value
            target_node -> deep_swap(replacing_node);
        }
        bool is_left_child = target_node -> is_left_child();
        node_type* target_parent = target_node -> parent;
        // RP stands for red parent. RU stands for red uncle. RG stands for red grandparent.
        // BP stands for black parent. BU stands for black uncle. BG stands for black grandparent
        // RT stands for red target node. BT stands for black current node.
        // RC stands for red child node. BC stands for black child node.
        // RS stands for red sibling node. BS stands for black sibling node.

        /* Since we assume the target node has at most one child,
         * and red node can't have red child, this node may only have a black child.
         * However the child can't be black because that would break the constraint of
         * "same number of black nodes along any path". Therefore, the target
         * node has no child and is safe to remove.
         *  
         *   P        P
         *   |   ->
         *   RT
         */
        if (node_type::is_red(target_node)) {
            target_node -> orphan_self();
        } else if (target_node -> left_child) {
            /* A black target node may only have a red child because otherwise,
             * the constraint of "same number of black along any path" will break for the target node
             *      P         P
             *      |         |
             *      BT   ->   BC
             *     /
             *    RC
             */
            target_node -> left_child -> mark_as_black();
            target_node -> left_child -> increment_height(1);
            target_parent -> link_child(target_node -> orphan_left_child(), is_left_child);
        } else if (target_node -> right_child) {
            /*
             *      P         P
             *      |         |
             *      BT   ->   BC
             *        \
             *        RC
             */
            target_node -> right_child -> mark_as_black();
            target_node -> right_child -> increment_height(1);
            target_parent -> link_child(target_node -> template orphan_right_child<false>(), is_left_child);
        } else {
            // curr represents the current subtree that just lost one black node along any path
            for (node_type* curr = target_node; curr -> parent != end;) {
                node_type* parent = curr -> parent;
                bool is_left_child = curr -> is_left_child();
                node_type* sibling = is_left_child ? parent -> right_child.get() : parent -> left_child.get();
                bool is_sibling_left_child = sibling -> is_left_child();
                /*
                 * Parent is black and sibling is red
                 * Make parent and sibling swap color and perform a rotation on the parent
                 * so the sibling takes parent's place
                 * 
                 * Note that the right child of sibling is not drawn. However,
                 * it should have the same black height as the target node
                 * because target node had the same black height as the sibling
                 * and lost one black node along the path to leaf. Losing the black node
                 * equalizes its height with that of the right child of the sibling.
                 * 
                 *      BP            RP            BS
                 *     / \           /  \          /
                 *    T   RS   ->   T   BS   ->   RP
                 *                               /
                 *                              T
                 */
                if (node_type::is_red(sibling)) {
                    parent -> mark_as_red();
                    parent -> increment_height(-1);
                    sibling -> mark_as_black();
                    sibling -> increment_height(1);
                    if (is_sibling_left_child) {
                        parent -> rotate_right();
                    } else {
                        parent -> rotate_left();
                    }
                } else {
                    // A black node may be null but the sibling can't be null in this case
                    // If the sibling is null, the equal black height constraint will be broken
                    // because curr is black
                    bool is_sibling_left_child_black = node_type::is_black(sibling -> left_child.get());
                    bool is_sibling_right_child_black = node_type::is_black(sibling -> right_child.get());
                    if (is_sibling_left_child_black && is_sibling_right_child_black) {
                        sibling -> mark_as_red();
                        sibling -> increment_height(-1);
                        if (node_type::is_red(parent)) {
                            /**
                             * Both children of the sibling are black. We can make sibling
                             * red without causing red red conflict. Paint parent as black
                             * to compensate black height loss on both branches
                             *      RP               BP
                             *     /  \             /  \
                             *    T   BS      ->   T   RS
                             *       /  \             /  \
                             *      B    B           B    B
                             */
                            parent -> mark_as_black();
                            break;
                        }
                        /**
                         * Both children of the sibling are black. We can make sibling
                         * red without causing red red conflict. The height loss needs
                         * to propagate up
                         *      BP               BP
                         *     /  \             /  \
                         *    T   BS      ->   T   RS
                         *       /  \             /  \
                         *      B    B           B    B
                         */
                        parent -> increment_height(-1);
                        curr = curr -> parent;
                    } else {
                        node_type* red_sibling_child =
                            is_sibling_left_child_black ? sibling -> right_child.get() : sibling -> left_child.get();
                        bool is_red_sibling_child_left = red_sibling_child -> is_left_child();
                        if (is_sibling_left_child) {
                            // sibling's right child is red
                            if (!is_red_sibling_child_left) {
                                /**
                                 * RSR stands for red sibling right child.
                                 * Note how the third stage matches the first stage in
                                 * the graph below the if block
                                 *      P              P               P
                                 *     / \            / \             / \
                                 *    BS  T          RS  T          BSR  T
                                 *   /  \     ->    /  \     ->    /  \
                                 *  B    RSR       B   BSR        RS   (B)
                                 *         \             \       /
                                 *          (B)          (B)    B
                                 */
                                red_sibling_child -> mark_as_black();
                                red_sibling_child -> increment_height(1);
                                sibling -> mark_as_red();
                                sibling -> increment_height(-1);
                                red_sibling_child = sibling;
                                sibling = sibling -> rotate_left();
                            }
                            /**
                             * 
                             *      P               S               S
                             *     / \             / \             / \
                             *    BS  T     ->    R   BP    ->    B   BP 
                             *   /  \             ^  /  \         ^  /  \
                             *  R    B              B    T          B    T
                             */
                            std::swap(sibling -> color_data, parent -> color_data);
                            red_sibling_child -> mark_as_black();
                            red_sibling_child -> increment_height(1);
                            parent -> rotate_right();
                        } else {
                            // sibling's left child is red
                            if (is_red_sibling_child_left) {
                                /**
                                 * RSL stands for red sibling left child.
                                 * Note how the third stage matches the first stage in
                                 * the graph below the if block
                                 *      P              P               P
                                 *     / \            / \             / \
                                 *    T  BS          T   RS         T   BSL
                                 *      /  \     ->     /  \   ->      /   \
                                 *    RSL   B         BSL   B        (B)    RS
                                 *    /               /                       \
                                 *  (B)             (B)                        B
                                 */
                                red_sibling_child -> mark_as_black();
                                red_sibling_child -> increment_height(1);
                                sibling -> mark_as_red();
                                sibling -> increment_height(-1);
                                red_sibling_child = sibling;
                                sibling = sibling -> rotate_right();
                            }
                            /**
                             * 
                             *      P               S               S
                             *     / \             / \             / \
                             *    T  BS     ->    BP  R    ->     BP   B 
                             *      /  \         /  \ ^          /  \  ^
                             *     B    R       T    B          T    B
                             */
                            std::swap(sibling -> color_data, parent -> color_data);
                            red_sibling_child -> mark_as_black();
                            red_sibling_child -> increment_height(1);
                            parent -> rotate_left();
                        }
                        break;
                    }
                }
            }
            target_node -> orphan_self();
        }
        return res;
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
        node_type* middle_node;
        if (!dest -> right_child) {
            middle_node = dest.release();
            dest.reset(middle_node -> orphan_left_child());
        } else {
            middle_node = dest -> get_rightmost_descendant();
            // dest may no longer be the root after rebalancing in extract
            // Releasing it to avoid having two unique pointers pointint owning the same raw pointer
            node_type* raw_dest = dest.release();
            extract(middle_node, nullptr);
            dest.reset(raw_dest -> get_root());
        }
        // We need to call join because dest may be shorter than src after removing the node
        return join(std::move(dest), unique_ptr_type(middle_node), std::move(src));
    }

    /**
     * @brief Join a tree into the right spine of another tree along with a middle node
     * 
     * All elements in the destination must be less than the middle node
     * All elements in the source must be greater than the middle node
     * 
     * @param dest a unique ptr to the tree to join into. Must be nonnull
     * @param src a unique ptr to the tree that will join the destination tree. Root must not be red
     *            Can be null
     * @param middle_node a unique ptr to the middle node. Must be nonnull
     * @return the result of the join
     */
    static unique_ptr_type join_right(unique_ptr_type dest, unique_ptr_type src, unique_ptr_type middle_node) {
        unsigned char max_balancing_height = src ? src -> get_black_height() : 0;
        node_type* curr = dest.get();
        node_type* parent = nullptr;
        while (curr && (curr -> get_black_height() > max_balancing_height ||
                (curr -> get_black_height() == max_balancing_height && node_type::is_red(curr)))) {
            parent = curr;
            curr = curr -> right_child.get();
        }

        node_type* middle_ptr = middle_node.release();
        node_type* raw_dest = dest.release();
        if (parent) {
            parent -> link_right_child(middle_ptr);
        }
        middle_ptr -> mark_as_red();
        middle_ptr -> safe_link_left_child(curr);
        middle_ptr -> safe_link_right_child(std::move(src));
        middle_ptr -> set_black_height(max_balancing_height);
        // Src may be red. If so, start solving red red conflict
        // from src
        fix_double_red(middle_ptr, nullptr);
        return unique_ptr_type(raw_dest -> parent ? raw_dest -> parent : raw_dest);
    }

    static unique_ptr_type join_left(unique_ptr_type dest, unique_ptr_type src) {
        if (!src) {
            return dest;
        }
        node_type* middle_node;
        if (!dest -> left_child) {
            middle_node = dest.release();
            dest.reset(middle_node -> orphan_right_child());
        } else {
            middle_node = dest -> get_leftmost_descendant();
            // dest may no longer be the root after rebalancing in extract
            // Releasing it to avoid having two unique pointers pointint owning the same raw pointer
            node_type* raw_dest = dest.release();
            extract(middle_node, nullptr);
            dest.reset(raw_dest -> get_root());
        }
        // We need to call join because dest may be shorter than src after removing the node
        return join(std::move(src), unique_ptr_type(middle_node), std::move(dest));
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
        unsigned char max_balancing_height = src ? src -> get_black_height() : 0;
        node_type* curr = dest.get();
        node_type* parent = nullptr;
        while (curr && (curr -> get_black_height() > max_balancing_height ||
                (curr -> get_black_height() == max_balancing_height && node_type::is_red(curr)))) {
            parent = curr;
            curr = curr -> left_child.get();
        }

        node_type* middle_ptr = middle_node.release();
        node_type* raw_dest = dest.release();
        if (parent) {
            parent -> link_left_child(middle_ptr);
        }
        middle_ptr -> mark_as_red();
        middle_ptr -> safe_link_left_child(std::move(src));
        middle_ptr -> safe_link_right_child(curr);
        middle_ptr -> set_black_height(max_balancing_height);
        fix_double_red(middle_ptr, nullptr);
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
            middle -> mark_as_red();
            middle -> set_black_height(0);
            return middle;
        }
        if (node_type::is_red(left.get())) {
            left -> mark_as_black();
            left -> increment_height(1);
        }
        if (node_type::is_red(right.get())) {
            right -> mark_as_black();
            right -> increment_height(1);
        }
        if (left && (!right || left -> get_black_height() >= right -> get_black_height())) {
            return join_right(std::move(left), std::move(right), std::move(middle));
        }
        return join_left(std::move(right), std::move(left), std::move(middle));
    }

    static unique_ptr_type join(unique_ptr_type left, unique_ptr_type right) {
        if (!left) {
            return right;
        }
        if (!right) {
            return left;
        }
        if (left -> get_black_height() >= right -> get_black_height()) {
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
                return make_pair(std::move(root), true);
            }
            divider -> left_child.reset(root -> orphan_left_child());
            divider -> right_child.reset(root -> orphan_right_child());
            root.release() -> destroy(node_allocator);
            return make_pair(std::move(divider), true);
        }
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
     * @param root the red black tree to split.
     * @param divider the node that contains the key.
     * @param resolver a conflict resolution function
     * @return a pair of node and boolean such that the node's left child and right child are complementary
     *         partitions of the input tree. Note that the returning node is no way a balance tree. It is just a way to return multiple things.
     *         without having to resort to tuple, which is slow. The boolean is true if a collision occurs. False otherwise.
     */
    template<typename Resolver=chooser<value_type> >
    std::pair<unique_ptr_type, bool> split(unique_ptr_type root, unique_ptr_type divider, Resolver resolver = Resolver()) {
        return split_helper(std::move(root), std::move(divider), key_of(divider -> value), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend red_black_tree union_of_helper(red_black_tree tree1, red_black_tree tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor, Resolver resolver) {
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
     * @brief Compute the union of two red black trees
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the tree
     *         It inputs two values and output true if the first value should be picked
     * @param tree1 the first operand of the union operation
     * @param tree2 the second operand of the union operation
     * @param resolver a conflict resolution function
     * @return the union of the trees
     */
    template<typename Resolver=chooser<value_type> >
    friend red_black_tree union_of(red_black_tree tree1, red_black_tree tree2, Resolver resolver=Resolver()) {
        return union_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>>(), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend red_black_tree union_of(red_black_tree tree1, red_black_tree tree2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) {
        return union_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>> (std::ref(executor)), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend red_black_tree intersection_of_helper(red_black_tree tree1, red_black_tree tree2,
        std::optional<std::reference_wrapper<thread_pool_executor>> executor, Resolver resolver) {
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
     * @brief Compute the intersection of two red black trees
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the tree
     *         It inputs two values and output true if the first value should be picked
     * @param tree1 the first operand of the union operation
     * @param tree2 the second operand of the union operation
     * @param resolver a conflict resolution function
     * @return the intersection of the trees
     */
    template<typename Resolver=chooser<value_type> >
    friend red_black_tree intersection_of(red_black_tree tree1, red_black_tree tree2, Resolver resolver=Resolver()) {
        return intersection_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>>(), resolver);
    }

    template<typename Resolver=chooser<value_type> >
    friend red_black_tree intersection_of(red_black_tree tree1, red_black_tree tree2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) {
        return intersection_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>> (std::ref(executor)), resolver);
    }

    friend red_black_tree difference_of_helper(red_black_tree tree1, red_black_tree tree2,
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
     * @brief Compute the difference of two red black trees
     * 
     * @param tree1 the tree to subtract from
     * @param tree2 the tree that subtracts
     * @return the difference of the trees
     */
    friend red_black_tree difference_of(red_black_tree tree1, red_black_tree tree2) {
        return difference_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>>());
    }

    friend red_black_tree difference_of(red_black_tree tree1, red_black_tree tree2, thread_pool_executor& executor) {
        return difference_of_helper(std::move(tree1), std::move(tree2),
            std::optional<std::reference_wrapper<thread_pool_executor>> (std::ref(executor)));
    }

    /**
     * @brief Swap content with another red black tree
     * 
     * @param other the other red black tree to swap from
     */
    void swap(red_black_tree& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        static_cast<base_type&>(*this).swap(static_cast<base_type&>(other));
    }

    /**
     * @brief Check equality of two red black trees
     * 
     * @param tree1 the first red black tree
     * @param tree2 the second red black tree
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const red_black_tree& tree1, const red_black_tree& tree2) requires equality_comparable<value_type> {
        return container_equals(tree1, tree2);
    }

    /**
     * @brief Compare two red black trees
     * 
     * @param tree1 the first red black tree
     * @param tree2 the second red black tree
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const red_black_tree& tree1, const red_black_tree& tree2) requires less_comparable<value_type> {
        return container_three_way_comparison(tree1, tree2);
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
    Compare key_comp() const noexcept {
        return comp;
    }

    /*
     * Testing purpose only
     */
    const red_black_node<V>* __get_sentinel() const noexcept {
        return sentinel;
    }

    base_type::node_allocator_type& __get_node_allocator() noexcept {
        return node_allocator;
    }

    static bool __has_no_consecutive_red_nodes_helper(const node_type* root) noexcept {
        if (!root) {
            return true;
        }
        if (node_type::is_red(root)) {
            if (root -> left_child && node_type::is_red(root -> left_child.get())) {
                return false;
            }
            if (root -> right_child && node_type::is_red(root -> right_child.get())) {
                return false;
            }
        }
        return __has_no_consecutive_red_nodes_helper(root -> left_child.get()) &&
                __has_no_consecutive_red_nodes_helper(root -> right_child.get()); 
    }

    bool __has_no_consecutive_red_nodes() const noexcept {
        return __has_no_consecutive_red_nodes_helper(sentinel -> left_child.get());
    }

    static int __are_all_black_paths_equally_long_helper(const node_type* root) noexcept {
        if (!root) {
            return 0;
        }
        int left_black_height = __are_all_black_paths_equally_long_helper(root -> left_child.get());
        if (left_black_height == -1) {
            return -1;
        }
        int right_black_height = __are_all_black_paths_equally_long_helper(root -> right_child.get());
        if (right_black_height == -1) {
            return -1;
        }
        if (left_black_height != right_black_height) {
            return -1;
        }
        int expected_black_height = left_black_height;
        if (node_type::is_black(root)) {
            expected_black_height += 1;
        }
        if (expected_black_height != root -> get_black_height()) {
            return -1;
        }
        return expected_black_height;
    }

    bool __are_all_black_paths_equally_long() const noexcept {
        return __are_all_black_paths_equally_long_helper(sentinel -> left_child.get()) != -1;
    }

    bool __is_valid() const noexcept {
        node_type* smallest = sentinel -> get_leftmost_descendant();
        return __is_inorder(const_iterator(smallest), this -> cend()) & __is_size_correct(sentinel -> left_child.get(), element_count) &
            __has_no_consecutive_red_nodes() &
            __are_all_black_paths_equally_long() &
            sentinel -> __is_parent_child_link_mutual() & 
            this -> __is_begin_node_correct();
    }
};
}
