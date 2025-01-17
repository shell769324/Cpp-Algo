#pragma once
#include "avl_tree.h"

namespace algo {
template<typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key> >
class avl_tree_set {
private:
    using avl_tree_type = avl_tree<Key, Key, std::identity, Compare, Allocator>;

public:
    using key_type = Key;
    using value_type = Key;
    using size_type = typename avl_tree_type::size_type;
    using difference_type = typename avl_tree_type::difference_type;
    using key_compare = typename avl_tree_type::key_compare;
    using value_compare = key_compare;
    using allocator_type = typename avl_tree_type::allocator_type;
    using reference = typename avl_tree_type::reference;
    using const_reference = typename avl_tree_type::const_reference;
    using pointer = typename avl_tree_type::pointer;
    using const_pointer = typename avl_tree_type::const_pointer;
    using iterator = typename avl_tree_type::iterator;
    using const_iterator = typename avl_tree_type::const_iterator;
    using reverse_iterator = typename avl_tree_type::reverse_iterator;
    using const_reverse_iterator = typename avl_tree_type::const_reverse_iterator;
    using node_type = typename avl_tree_type::node_type;

private:
    avl_tree_type tree;

public:
    /**
     * @brief Construct an empty avl tree set with the default comparator
     */
    avl_tree_set() = default;

    /**
     * @brief Construct an empty avl tree set with a comparator and an optional allocator
     * 
     * @param comp the value comparator used by the set
     * @param allocator the allocator to construct and destroy elements
     */
    explicit avl_tree_set(const Compare& comp, const Allocator& allocator = Allocator()) : tree(comp, allocator) { }

    /**
     * @brief Construct an empty avl tree set with an allocator
     * 
     * @param allocator the allocator to construct and destroy elements
     */
    explicit avl_tree_set(const Allocator& allocator) : tree(Compare(), allocator) { }

    /**
     * @brief Construct a new avl tree set from a range with an optional compare function and an optional allocator
     * 
     * @tparam InputIt the type of the iterators that define the range
     * @param first iterator to the first element in the range
     * @param last iterator to one past the last element in the range
     * @param comp the value comparator used by the set
     * @param allocator the allocator to construct and destroy elements
     */
    template<typename InputIt>
    avl_tree_set(InputIt first, InputIt last, 
                 const Compare& comp = Compare(),
                 const Allocator& allocator = Allocator()) 
        requires conjugate_uninitialized_input_iterator<InputIt, value_type> 
        : tree(first, last, comp, allocator) { }
    
    /**
     * @brief Construct a new avl tree set from a range with an allocator
     * 
     * @tparam InputIt the type of the iterators that define the range
     * @param first iterator to the first element in the range
     * @param last iterator to one past the last element in the range
     * @param allocator the allocator to construct and destroy elements
     */
    template<typename InputIt>
    avl_tree_set(InputIt first, InputIt last, const Allocator& allocator)
        requires conjugate_uninitialized_input_iterator<InputIt, value_type>
        : tree(first, last, Compare(), allocator) { }

    /**
     * @brief Construct a copy of another avl tree set
     * 
     * Copy constructor
     * 
     * @param other the set to copy from
     */
    avl_tree_set(const avl_tree_set& other) = default;

    /**
     * @brief Construct a copy of another avl tree set with an allocator
     * 
     * @param other the set to copy from
     * @param allocator the allocator to construct and destroy key value pairs
     */
    avl_tree_set(const avl_tree_set& other, const Allocator& allocator)
        : tree(other.tree, allocator) { }

    /**
     * @brief Construct a copy of another avl tree set by move
     * 
     * Move constructor
     * 
     * @param other the set to move from
     */
    avl_tree_set(avl_tree_set&& other) = default;

    /**
     * @brief Construct a copy of another avl tree set with an allocator by move
     * 
     * @param other the set to move from
     * @param allocator the allocator to construct and destroy key value pairs
     */
    avl_tree_set(avl_tree_set&& other, const Allocator& allocator)
        : tree(std::move(other.tree), allocator) { }

    /**
     * @brief Destroy the avl tree set
     */
    ~avl_tree_set() noexcept { } 

    /**
     * @brief Copy assignment operator
     * 
     * @param other the set to copy from
     * @return a reference to this set
     */
    avl_tree_set& operator=(const avl_tree_set& other) {
        if (this == &other) {
            return *this;
        }
        avl_tree_set tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Move assignment operator
     * 
     * @param other the set to move from
     * @return a reference to this set
     */
    avl_tree_set& operator=(avl_tree_set&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief Get a copy of the associated allocator
     */
    allocator_type get_allocator() const noexcept {
        return tree.get_allocator();
    }

    /**
     * @brief Get an iterator to the smallest element
     */
    iterator begin() noexcept {
        return tree.begin();
    }

    /**
     * @brief Get a const iterator to the smallest element
     */
    const_iterator begin() const noexcept {
        return tree.begin();
    }

    /**
     * @brief Get a const iterator to the smallest element
     */
    const_iterator cbegin() const noexcept {
        return tree.cbegin();
    }

    /**
     * @brief Get an iterator to the element following the greatest element
     */
    iterator end() noexcept {
        return tree.end();
    }

    /**
     * @brief Get a const iterator to the element following the greatest element
     */
    const_iterator end() const noexcept {
        return tree.end();
    }

    /**
     * @brief Get a const iterator to the element following the greatest element
     */
    const_iterator cend() const noexcept {
        return tree.cend();
    }

    /**
     * @brief Get a reverse iterator to the greatest element
     */
    reverse_iterator rbegin() noexcept {
        return tree.rbegin();
    }

    /**
     * @brief Get a const reverse iterator to the greatest element
     */
    const_reverse_iterator rbegin() const noexcept {
        return tree.rbegin();
    }

    /**
     * @brief Get a const reverse iterator to the greatest element
     */
    const_reverse_iterator crbegin() const noexcept {
        return tree.crbegin();
    }

    /**
     * @brief Get an iterator to the element before the smallest element
     */
    reverse_iterator rend() noexcept {
        return tree.rend();
    }

    /**
     * @brief Get a const iterator to the element before the smallest element
     */
    const_reverse_iterator rend() const noexcept {
        return tree.rend();
    }

    /**
     * @brief Get a const iterator to the element before the smallest element
     */
    virtual const_reverse_iterator crend() const noexcept {
        return tree.crend();
    }

    /**
     * @brief Test if this set has no elements
     */
    [[nodiscard]] bool empty() const noexcept {
        return tree.empty();
    }

    /**
     * @brief Get the number of elements
     */
    [[nodiscard]] std::size_t size() const noexcept {
        return tree.size();
    }

    /**
     * @brief Remove all elements
     */
    void clear() noexcept {
        tree.clear();
    }

    /**
     * @brief Insert an element by copy
     * 
     * @param value the value to insert
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted element, or the existing one if this element already exists
     * 
     * The boolean is true if insertion succeeded, namely the element is not found, false otherwise
     */
    std::pair<iterator, bool> insert(const value_type& value) {
        return tree.insert(value);
    }

    /**
     * @brief Insert an element by move copy
     * 
     * @param value the value to insert
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this element already exists
     * 
     * The boolean is true if insertion succeeded, namely the element is not found, false otherwise
     */
    std::pair<iterator, bool> insert(value_type&& value) {
        return tree.insert(std::move(value));
    }

    /**
     * @brief Insert a single value to this set
     * 
     * @param pos   an iterator that is close to the insertion location of the new value. If the iterator
     *              is far from insertion location, the value is inserted as if insert(value)
     * @param value the value to be copied and inserted
     * 
     * @return an iterator to the inserted element, or to the element that prevented the insertion
     */
    iterator insert(const_iterator pos, const value_type& value) requires std::is_copy_constructible_v<value_type> {
        return tree.insert(pos, value);
    }

    /**
     * @brief Insert a single value to this set
     * 
     * @param pos   an iterator that is close to the insertion location of the new value. If the iterator
     *              is far from insertion location, the value is inserted as if insert(std::move(value))
     * @param value the value to be moved and inserted
     * 
     * @return an iterator to the inserted element, or to the element that prevented the insertion
     */
    iterator insert(const_iterator pos, value_type&& value) requires std::move_constructible<value_type> {
        return tree.insert(pos, std::move(value));
    }

    /**
     * @brief Insert all elements in a range
     * 
     * @tparam InputIt the type of the iterator
     * @param first an iterator to the first element in the range
     * @param last an iterator to one past the last element in the range
     */
    template <typename InputIt>
    void insert(InputIt first, InputIt last) requires conjugate_uninitialized_input_iterator<InputIt, value_type> {
        tree.insert(first, last);
    }

    /**
     * @brief In-place construct and insert an element given the arguments
     *        to construct it
     * 
     * The element is always constructed, regardless if the element already exists
     * 
     * @tparam the type of arguments to construct the element
     * @param args the arguments to construct the element
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this element already exists
     * 
     * The boolean is true if insertion succeeded, namely the element is not found, false otherwise
     */
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return tree.emplace(std::forward<Args>(args)...);
    }

    /**
     * @brief In-place construct and insert an element given the arguments
     *        to construct it. Use an iterator as a hint for efficient insertion.
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
        return tree.emplace_hint(hint, std::forward<Args>(args)...);
    }

    /**
     * @brief Remove the element pointed by a given iterator
     * 
     * @param pos a iterator that points to the element to remove
     * @return the iterator to the successor of the removed element
     */
    iterator erase(iterator pos) {
        return tree.erase(pos);
    }

    /**
     * @brief Remove the element pointed by a given iterator
     * 
     * @param pos a const iterator that points to the element to remove
     * @return the iterator to the successor of the removed element
     */
    iterator erase(const_iterator pos) {
        return tree.erase(pos);
    }

    /**
     * @brief Remove all elements in a range
     * 
     * @param first the iterator to the first element in the range
     * @param last the iterator to one past the last element in the range
     * @return the iterator to the successor of the last removed element
     */
    iterator erase(iterator first, iterator last) {
        return tree.erase(first, last);
    }
    
    /**
     * @brief Remove an element
     * 
     * @param value the value to erase
     * @return true if removal happened (i.e. the element was found), false otherwise
     */
    bool erase(const value_type& value) {
        return tree.erase(value);
    }

    /**
     * @brief Swap content with another tree
     * 
     * @param other the tree to swap content with
     */
    void swap(avl_tree_set& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        tree.swap(other.tree);
    }

    /**
     * @brief Check equality of two avl tree sets
     * 
     * @param set1 the first avl tree set
     * @param set2 the second avl tree set
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const avl_tree_set& set1, const avl_tree_set& set2) requires equality_comparable<value_type> {
        return set1.tree == set2.tree;
    }

    /**
     * @brief Compare two avl tree sets
     * 
     * @param set1 the first avl tree set
     * @param set2 the second avl tree set
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const avl_tree_set& set1, const avl_tree_set& set2) requires less_comparable<value_type> {
        return set1.tree <=> set2.tree;
    }

    /**
     * @brief count the number of occurrences of a particular value
     * 
     * @param value the value to query
     * @return 1 if this value exists in this set, 0 otherwise
     */
    std::size_t count(const value_type& value) {
        return tree.find(value) != tree.end() ? 1 : 0;
    }
    
    /**
     * @brief Get the iterator to an element
     * 
     * @param value the element to query
     * @return an iterator to the element if it is found
     *         an iterator equivalent to end() otherwise
     */
    iterator find(const value_type& value) {
        return tree.find(value);
    }

    /**
     * @brief Get the iterator to an element
     * 
     * @param value the element to query
     * @return a const iterator to the element if it is found
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator find(const value_type& value) const {
        return tree.find(value);
    }

    /**
     * @brief Test if an element exists
     * 
     * @param value the element to query
     * @return true if the value is found, false otherwise
     */
    bool contains(const value_type& value) const {
        return tree.find(value) != tree.end();
    }

    /**
     * @brief Get the iterator to the smallest element greater than the given one
     * 
     * @param value the element to query
     * @return an iterator to such element if it exists
     *         an iterator equivalent to end() otherwise
     */
    iterator upper_bound(const value_type& value) {
        return tree.upper_bound(value);
    }

    /**
     * @brief Get the iterator to the smallest element greater than the given one
     * 
     * @param value the element to query
     * @return a const iterator to such element if it exists
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator upper_bound(const value_type& value) const {
        return tree.upper_bound(value);
    }

    /**
     * @brief Get the iterator to the smallest element greater than
     *        or equal to a given one
     * 
     * @param value the element to query
     * @return an iterator to such element if it exists
     *         an iterator equivalent to end() otherwise
     */
    iterator lower_bound(const value_type& value) {
        return tree.lower_bound(value);
    }

    /**
     * @brief Get the iterator to the smallest element greater than
     *        or equal to a given one
     * 
     * @param value the element to query
     * @return a const iterator to such element if it exists
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator lower_bound(const value_type& value) const {
        return tree.lower_bound(value);
    }

    /**
     * @brief Compute the union of two avl tree sets
     * 
     * @tparam Resolver a function that resolves conflicts if the value also exists in the set
     *         It takes in two values and output true if the first value should be picked
     * @param set1 the first operand of the union operation
     * @param set2 the second operand of the union operation
     * @param resolver a conflict resolution function
     * @return the union of the avl tree sets 
     */
    template<typename Resolver=chooser<value_type> >
    friend avl_tree_set union_of(avl_tree_set set1, avl_tree_set set2, Resolver resolver=Resolver())
        requires is_resolver<value_type, Resolver> {
        set1.tree = union_of(std::move(set1.tree), std::move(set2.tree), resolver);
        return set1;
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree_set union_of(avl_tree_set set1, avl_tree_set set2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) requires is_resolver<value_type, Resolver> {
        set1.tree = union_of(std::move(set1.tree), std::move(set2.tree), executor, resolver);
        return set1;
    }

    /**
     * @brief Compute the intersection of two avl tree sets
     * 
     * @tparam Resolver a function that resolves conflicts if the value also exists in the set
     *         It takes in two values and output true if the first value should be picked
     * @param set1 the first operand of the intersection operation
     * @param set2 the second operand of the intersection operation
     * @param resolver a conflict resolution function
     * @return the intersection of the avl tree sets 
     */
    template<typename Resolver=chooser<value_type> >
    friend avl_tree_set intersection_of(avl_tree_set set1, avl_tree_set set2, Resolver resolver=Resolver())
        requires is_resolver<value_type, Resolver> {
        set1.tree = intersection_of(std::move(set1.tree), std::move(set2.tree), resolver);
        return set1;
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree_set intersection_of(avl_tree_set set1, avl_tree_set set2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) requires is_resolver<value_type, Resolver>  {
        set1.tree = intersection_of(std::move(set1.tree), std::move(set2.tree), executor, resolver);
        return set1;
    }

    /**
     * @brief Compute the difference of two avl tree sets
     * 
     * @param set1 the set to subtract from
     * @param set2 the set that subtracts
     * @return the difference of the avl tree sets 
     */
    friend avl_tree_set difference_of(avl_tree_set set1, avl_tree_set set2) {
        set1.tree = difference_of(std::move(set1.tree), std::move(set2.tree));
        return set1;
    }

    friend avl_tree_set difference_of(avl_tree_set set1, avl_tree_set set2, thread_pool_executor& executor) {
        set1.tree = difference_of(std::move(set1.tree), std::move(set2.tree), executor);
        return set1;
    }

    /**
     * @brief Get the value comparison object
     */
    value_compare value_comp() const {
        return tree.key_comp();
    }

    // For testing purpose
    bool __is_valid() const noexcept {
        return tree.__is_valid();
    }
};
}
