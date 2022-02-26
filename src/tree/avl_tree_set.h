#pragma once
#include <functional>
#include "binary_tree_iterator.h"
#include "avl_tree.h"

namespace algo {
template<typename T, typename Compare = std::less<T> >
class avl_tree_set {
public:

    using value_type = T;
    using value_compare = Compare;
    using reference = T&;
    using const_reference = const T&;
    using iterator = binary_tree_iterator<value_type>;
    using const_iterator = binary_tree_iterator<const value_type>;
    using reverse_iterator = binary_tree_iterator<value_type, true>;
    using const_reverse_iterator = binary_tree_iterator<const value_type, true>;

private:
    avl_tree<value_type, value_type, std::identity, Compare> tree;

public:
    
    /**
     * @brief Construct an empty avl tree set with default comparator
     */
    avl_tree_set() = default;

    /**
     * @brief Construct an empty avl tree set with a given comparator
     * 
     * @param comp the value comparator used by the set
     *             will be copy constructed
     */
    avl_tree_set(const Compare& comp) : tree(comp) { }

    /**
     * @brief Construct an empty avl tree set with a given comparator
     * 
     * @param comp the value comparator used by the tree
     *             will be move constructed
     */
    avl_tree_set(Compare&& comp) : tree(comp) { }

    /**
     * @brief Construct a copy of another avl tree set
     * 
     * Copy constructor
     * 
     * @param other the set to copy from
     */
    avl_tree_set(const avl_tree_set& other) = default;

    /**
     * @brief Construct a copy of another avl tree set
     * 
     * Move constructor
     * 
     * @param other the set to move from
     */
    avl_tree_set(avl_tree_set&& other) = default;

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
     * @brief Construct a new avl tree set from a range
     * 
     * @tparam InputIt the type of the iterators that define the range
     * @param first iterator to the first element in the range
     * @param last iterator to one past the last element in the range
     */
    template<std::input_iterator InputIt>
    avl_tree_set(InputIt first, InputIt last) : avl_tree_set() {
        insert(first, last);
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
    [[nodiscard]] bool is_empty() const noexcept {
        return tree.is_empty();
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
     * @brief Insert all elements in a range
     * 
     * @tparam InputIt the type of the iterator
     * @param first an iterator to the first element in the range
     * @param last an iterator to one past the last element in the range
     */
    template <std::input_iterator InputIt>
    void insert(InputIt first, InputIt last) {
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
     * @brief Get the iterator to the greatest element less than
     *        or equal to a given one
     * 
     * @param value the element to query
     * @return an iterator to such element if it exists
     *         an iterator equivalent to end() otherwise
     */
    iterator max_leq(const value_type& value) {
        return tree.max_leq(value);
    }

    /**
     * @brief Get the iterator to the greatest element less than
     *        or equal to a given one
     * 
     * @param value the element to query
     * @return a const iterator to such element if it exists
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator max_leq(const value_type& value) const {
        return tree.max_leq(value);
    }

    /**
     * @brief Get the iterator to the smallest element greater than
     *        or equal to a given one
     * 
     * @param value the element to query
     * @return an iterator to such element if it exists
     *         an iterator equivalent to end() otherwise
     */
    iterator min_geq(const value_type& value) {
        return tree.min_geq(value);
    }

    /**
     * @brief Get the iterator to the smallest element greater than
     *        or equal to a given one
     * 
     * @param value the element to query
     * @return a const iterator to such element if it exists
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator min_geq(const value_type& value) const {
        return tree.min_geq(value);
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
    friend avl_tree_set union_of(avl_tree_set set1, avl_tree_set set2, Resolver resolver=Resolver()) {
        set1.tree = union_of(std::move(set1.tree), std::move(set2.tree), resolver);
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
    friend avl_tree_set intersection_of(avl_tree_set set1, avl_tree_set set2, Resolver resolver=Resolver()) {
        set1.tree = intersection_of(std::move(set1.tree), std::move(set2.tree), resolver);
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

    /**
     * @brief Get the value comparison object
     */
    value_compare value_comp() const {
        return tree.get_comparator();
    }

    // For testing purpose
    bool is_valid() const noexcept {
        return tree.is_valid();
    }
};
}
