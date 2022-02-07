#pragma once
#include <functional>
#include "tree/binary_tree_iterator.h"

namespace algo {
template<typename T, typename Compare = std::less<T> >
class set {
public:
    using value_type = T;
    using value_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = binary_tree_iterator<value_type>;
    using const_iterator = binary_tree_iterator<const value_type>;
    using reverse_iterator = binary_tree_iterator<value_type, true>;
    using const_reverse_iterator = const binary_tree_iterator<const value_type, true>;

    virtual ~set() noexcept { }

    // Iterators

    /**
     * @brief Get the iterator to the first element in the set
     */
    virtual iterator begin() noexcept = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator cbegin() const noexcept = 0;

    /**
     * @brief Get the iterator to one past the last element in the set
     */
    virtual iterator end() noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual const_iterator cend() const noexcept = 0;

    /**
     * @brief Get the reverse iterator to the last element in the set
     */
    virtual reverse_iterator rbegin() noexcept = 0;
    virtual const_reverse_iterator rbegin() const noexcept = 0;
    virtual const_reverse_iterator crbegin() const noexcept = 0;

    /**
     * @brief Get the reverse iterator to one before the first element in the set
     */
    virtual reverse_iterator rend() noexcept = 0;
    virtual const_reverse_iterator rend() const noexcept = 0;
    virtual const_reverse_iterator crend() const noexcept = 0;

    // Capacity
    /**
     * @brief Test if the set is empty
     */
    [[nodiscard]] virtual bool is_empty() const noexcept = 0;

    /**
     * @brief Get the number of elements in the set
     */
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;

    // Modifiers
    /**
     * @brief Remove all elements in the set
     */
    virtual void clear() noexcept = 0;

    /**
     * @brief Insert an element to the set
     * 
     * @param value the value to insert
     * @return std::pair<iterator, bool> a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted element, or the existing one if this element already exists
     * 
     * The boolean is true if insertion succeeded, namely the element is not a duplicate, false otherwise
     */
    virtual std::pair<iterator, bool> insert(const value_type& value) = 0;
    virtual std::pair<iterator, bool> insert(value_type&& value) = 0;

    /**
     * @brief Remove the element pointed by a given iterator
     * 
     * @param pos the iterator that points to the element to remove
     * @return the iterator following the removed element
     */
    virtual iterator erase(iterator pos) = 0;
    virtual iterator erase(const_iterator pos) = 0;

    /**
     * @brief Remove all elements in a range
     * 
     * @param first the iterator to the first element in the range
     * @param last the iterator to one past the last element in the range
     * @return the iterator following the removed element
     */
    virtual iterator erase(iterator first, iterator last) = 0;
    
    // Lookup

    /**
     * @brief Get the iterator to an element
     * 
     * @param value the value to look up
     * @return an iterator to the element if it exists,
     *         the end iterator otherwise
     */
    virtual iterator find(const value_type& value) = 0;
    virtual const_iterator find(const value_type& value) const = 0;

    /**
     * @brief Test if an element exists in the set
     * 
     * @param value the value to look up
     * @return true if it exists, false otherwise
     */
    virtual bool contains(const value_type& value) const = 0;

    /**
     * @brief Get the iterator to the greatest element less than or equal to
     *        a given value
     * 
     * @param value the inclusive upper bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    virtual iterator max_leq(const value_type& value) = 0;
    virtual const_iterator max_leq(const value_type& value) const = 0;
    
    /**
     * @brief Get the iterator to the smallest element greater than or equal to
     *        a given value
     * 
     * @param value the inclusive lower bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    virtual iterator min_geq(const value_type& value) = 0;
    virtual const_iterator min_geq(const value_type& value) const = 0;

    /**
     * @brief Get a copy of the value comparator
     */
    virtual value_compare value_comp() const = 0;
};
}