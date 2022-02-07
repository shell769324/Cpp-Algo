#pragma once
#include <functional>
#include "binary_tree_iterator.h"
#include "src/set.h"
#include "avl_tree.h"

namespace algo {
template<typename T, typename Compare = std::less<T> >
class avl_tree_set : public set<T, Compare> {
public:
    using parent_type = set<T, Compare>;

    using typename parent_type::value_type;
    using typename parent_type::value_compare;
    using typename parent_type::reference;
    using typename parent_type::const_reference;
    using typename parent_type::iterator;
    using typename parent_type::const_iterator;
    using typename parent_type::reverse_iterator;
    using typename parent_type::const_reverse_iterator;

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
    ~avl_tree_set() noexcept override { } 

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

    // Iterators
    iterator begin() noexcept {
        return tree.begin();
    }

    const_iterator begin() const noexcept {
        return tree.begin();
    }

    const_iterator cbegin() const noexcept {
        return tree.cbegin();
    }

    iterator end() noexcept {
        return tree.end();
    }

    const_iterator end() const noexcept {
        return tree.end();
    }
    const_iterator cend() const noexcept {
        return tree.cend();
    }

    reverse_iterator rbegin() noexcept {
        return tree.rbegin();
    }

    const_reverse_iterator rbegin() const noexcept {
        return tree.rbegin();
    }

    const_reverse_iterator crbegin() const noexcept {
        return tree.crbegin();
    }

    reverse_iterator rend() noexcept {
        return tree.rend();
    }

    const_reverse_iterator rend() const noexcept {
        return tree.rend();
    }

    const_reverse_iterator crend() const noexcept {
        return tree.crend();
    }

    // Capacity
    [[nodiscard]] bool is_empty() const noexcept {
        return tree.is_empty();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return tree.size();
    }

    // Modifiers
    void clear() noexcept {
        tree.clear();
    }

    std::pair<iterator, bool> insert(const value_type& value) {
        return tree.insert(value);
    }
    std::pair<iterator, bool> insert(value_type&& value) {
        return tree.insert(std::move(value));
    }

    iterator erase(iterator pos) {
        return tree.erase(pos);
    }

    iterator erase(const_iterator pos) {
        return tree.erase(pos);
    }

    iterator erase(iterator first, iterator last) {
        return tree.erase(first, last);
    }
    
    // Lookup
    iterator find(const value_type& value) {
        return tree.find(value);
    }
    
    const_iterator find(const value_type& value) const {
        return tree.find(value);
    }

    bool contains(const value_type& value) const {
        return tree.find(value) != tree.end();
    }

    iterator max_leq(const value_type& value) {
        return tree.max_leq(value);
    }

    const_iterator max_leq(const value_type& value) const {
        return tree.max_leq(value);
    }
    
    iterator min_geq(const value_type& value) {
        return tree.min_geq(value);
    }

    const_iterator min_geq(const value_type& value) const {
        return tree.min_geq(value);
    }

    void swap(avl_tree_set& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        tree.swap(other.tree);
    }

    value_compare value_comp() const {
        return tree.get_comparator();
    }
};
}