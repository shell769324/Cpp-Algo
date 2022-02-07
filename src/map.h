#pragma once
#include <functional>
#include "tree/binary_tree_iterator.h"

namespace algo {
template<typename Key, typename T, typename Compare = std::less<Key> >
class map {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using key_compare = Compare;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = binary_tree_iterator<value_type>;
    using const_iterator = binary_tree_iterator<const value_type>;
    using reverse_iterator = binary_tree_iterator<value_type, true>;
    using const_reverse_iterator = const binary_tree_iterator<const value_type, true>;

    virtual ~map() noexcept { }
    
    // Element access

    /**
     * @brief Look up the value given a key
     * 
     * Throw out_of_range if key is not found
     * 
     * @param key the key associated with the value
     * @return the value associated with the key
     */
    virtual T& at(const key_type& key) = 0;   
    virtual const T& at(const key_type& key) const = 0;

    /**
     * @brief Look up the value given a key
     * 
     * Default construct a value for the key if it is not found
     * 
     * @param key the key associated with the value
     * @return the value associated with the key if it exists
     *         a default constructed value otherwise
     */
    virtual T& operator[](const key_type& key) = 0;
    virtual T& operator[](key_type&& key) = 0;

    // Iterators

    /**
     * @brief Get the iterator to the first element in the map
     */
    virtual iterator begin() noexcept = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator cbegin() const noexcept = 0;

    /**
     * @brief Get the iterator to one past the last element in the map
     */
    virtual iterator end() noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual const_iterator cend() const noexcept = 0;

    /**
     * @brief Get the reverse iterator to the last element in the map
     */
    virtual reverse_iterator rbegin() noexcept = 0;
    virtual const_reverse_iterator rbegin() const noexcept = 0;
    virtual const_reverse_iterator crbegin() const noexcept = 0;

    /**
     * @brief Get the reverse iterator to one before the first element in the map
     */
    virtual reverse_iterator rend() noexcept = 0;
    virtual const_reverse_iterator rend() const noexcept = 0;
    virtual const_reverse_iterator crend() const noexcept = 0;

    // Capacity
    /**
     * @brief Test if the map is empty
     */
    [[nodiscard]] virtual bool is_empty() const noexcept = 0;

    /**
     * @brief Get the number of key value pairs in the map
     */
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;

    // Modifiers
    /**
     * @brief Remove all elements in the map
     */
    virtual void clear() noexcept = 0;

    /**
     * @brief Insert a key value pair to the map
     * 
     * @param value a key value pair
     * @return std::pair<iterator, bool> a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted value, or the existing one if this value already exists
     * 
     * The boolean is true if insertion succeeded, namely the value is not a duplicate, false otherwise
     */
    virtual std::pair<iterator, bool> insert(const value_type& value) = 0;
    virtual std::pair<iterator, bool> insert(value_type&& value) = 0;

    /**
     * @brief Remove the value pointed by a given iterator
     * 
     * @param pos the iterator that points to the value to remove
     * @return the iterator following the removed element
     */
    virtual iterator erase(iterator pos) = 0;
    virtual iterator erase(const_iterator pos) = 0;

    /**
     * @brief Remove all values in a range
     * 
     * @param first the iterator to the first element in the range
     * @param last the iterator to one past the last element in the range
     * @return the iterator following the removed element
     */
    virtual iterator erase(iterator first, iterator last) = 0;

    /**
     * @brief Remove the value associated with a given key
     * 
     * @param key the key of the value to erase
     * @return true if removal happened (i.e. the key was found), false otherwise
     */
    virtual bool erase(const key_type& key) = 0;
    
    // Lookup

    /**
     * @brief Get the iterator to an element given a key
     * 
     * @param key the key to look up
     * @return an iterator to the element if it exists,
     *         the end iterator otherwise
     */
    virtual iterator find(const key_type& key) = 0;
    virtual const_iterator find( const key_type& key ) const = 0;

    /**
     * @brief Test if a key exists in the map
     * 
     * @param key the key to look up
     * @return true if it exists, false otherwise
     */
    virtual bool contains(const key_type& key) const = 0;

    /**
     * @brief Get the iterator to the greatest element less than or equal to
     *        a given key
     * 
     * @param key the inclusive upper bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    virtual iterator max_leq(const key_type& key) = 0;
    virtual const_iterator max_leq(const key_type& key) const = 0;
    
    /**
     * @brief Get the iterator to the smallest element greater than or equal to
     *        a given key
     * 
     * @param key the inclusive lower bound
     * @return an iterator to such element if it exists,
     *         the end iterator otherwise
     */
    virtual iterator min_geq(const key_type& key) = 0;
    virtual const_iterator min_geq(const key_type& key) const = 0;

    /**
     * @brief Get a copy of the key comparator
     */
    virtual key_compare key_comp() const = 0;
};
}