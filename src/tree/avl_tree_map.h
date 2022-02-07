#pragma once
#include <functional>
#include "src/map.h"
#include "src/common.h"
#include "avl_tree.h"

namespace algo {
template<typename Key, typename T, typename Compare = std::less<Key> >
class avl_tree_map : public map<Key, T, Compare> {

public:
    using parent_type = map<Key, T, Compare>;

    using typename parent_type::key_type;
    using typename parent_type::mapped_type;
    using typename parent_type::value_type;
    using typename parent_type::key_compare;
    using typename parent_type::reference;
    using typename parent_type::const_reference;
    using typename parent_type::iterator;
    using typename parent_type::const_iterator;
    using typename parent_type::reverse_iterator;
    using typename parent_type::const_reverse_iterator;

private:
    avl_tree<key_type, value_type, pair_left_accessor<key_type, mapped_type>, Compare> tree;

public:
    
    /**
     * @brief Construct an empty avl tree map with default comparator
     */
    avl_tree_map() = default;

    /**
     * @brief Construct an empty avl tree map with a given comparator
     * 
     * @param comp the key comparator used by the map
     *             will be copy constructed
     */
    avl_tree_map(const Compare& comp) : tree(comp) { }

    /**
     * @brief Construct an empty avl tree map with a given comparator
     * 
     * @param comp the key comparator used by the tree
     *             will be move constructed
     */
    avl_tree_map(Compare&& comp) : tree(comp) { }

    /**
     * @brief Construct a copy of another avl tree map
     * 
     * Copy constructor
     * 
     * @param other the map to copy from
     */
    avl_tree_map(const avl_tree_map& other) = default;

    /**
     * @brief Construct a copy of another avl tree map
     * 
     * Move constructor
     * 
     * @param other the map to move from
     */
    avl_tree_map(avl_tree_map&& other) = default;

    /**
     * @brief Destroy the avl tree map
     */
    ~avl_tree_map() noexcept override { } 

    /**
     * @brief Copy assignment operator
     * 
     * @param other the map to copy from
     * @return a reference to this map
     */
    avl_tree_map& operator=(const avl_tree_map& other) {
        if (this == &other) {
            return *this;
        }
        avl_tree_map tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Move assignment operator
     * 
     * @param other the map to move from
     * @return a reference to this map
     */
    avl_tree_map& operator=(avl_tree_map&& other) noexcept {
        swap(other);
        return *this;
    }

    // Element access
    T& at(const key_type& key) override {
        iterator it = tree.find(key);
        if (it == tree.end()) {
            throw std::out_of_range("Key not found");
        }
        return (*it).second;
    }

    const T& at(const key_type& key) const override {
        const_iterator it = tree.find(key);
        if (it == tree.end()) {
            throw std::out_of_range("Key not found");
        }
        return (*it).second;
    }

    T& operator[](const key_type& key) override {
        std::pair<iterator, bool> emplace_result = tree.try_emplace(key);
        return (*emplace_result.first).second;
    }

    T& operator[](key_type&& key) override {
        std::pair<iterator, bool> emplace_result = tree.try_emplace(std::move(key));
        return (*emplace_result.first).second;
    }

    // Iterators
    iterator begin() noexcept override {
        return tree.begin();
    }

    const_iterator begin() const noexcept override {
        return tree.begin();
    }

    const_iterator cbegin() const noexcept override {
        return tree.cbegin();
    }

    iterator end() noexcept override {
        return tree.end();
    }

    const_iterator end() const noexcept override {
        return tree.end();
    }
    const_iterator cend() const noexcept override {
        return tree.cend();
    }

    reverse_iterator rbegin() noexcept override {
        return tree.rbegin();
    }
    const_reverse_iterator rbegin() const noexcept override {
        return tree.rbegin();
    }
    
    const_reverse_iterator crbegin() const noexcept override {
        return tree.crbegin();
    }

    reverse_iterator rend() noexcept override {
        return tree.rend();
    }

    const_reverse_iterator rend() const noexcept override {
        return tree.rend();
    }

    virtual const_reverse_iterator crend() const noexcept override {
        return tree.crend();
    }

    // Capacity
    [[nodiscard]] bool is_empty() const noexcept override {
        return tree.is_empty();
    }

    [[nodiscard]] std::size_t size() const noexcept override {
        return tree.size();
    }

    // Modifiers
    void clear() noexcept override {
        tree.clear();
    }

    std::pair<iterator, bool> insert(const value_type& value) override {
        return tree.insert(value);
    }

    std::pair<iterator, bool> insert(value_type&& value) override {
        return tree.insert(value);
    }

    template <std::input_iterator InputIt>
    void insert(InputIt first, InputIt last) {
        tree.insert(first, last);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return tree.emplace(std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
        return tree.try_emplace(key, std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
        return tree.try_emplace(std::move(key), std::forward<Args>(args)...);
    }

    iterator erase(iterator pos) override {
        return tree.erase(pos);
    }

    iterator erase(const_iterator pos) override {
        return tree.erase(pos);
    }

    iterator erase(iterator first, iterator last) override {
        return tree.erase(first, last);
    }
    
    bool erase(const key_type& key) override {
        return tree.erase(key);
    }

    void swap(avl_tree_map& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        tree.swap(other.tree);
    }
    
    // Lookup
    iterator find(const key_type& key) override {
        return tree.find(key);
    }

    const_iterator find(const key_type& key) const override {
        return tree.find(key);
    }

    bool contains(const key_type& key) const override {
        return tree.find(key) != tree.end();
    }

    iterator max_leq(const key_type& key) override {
        return tree.max_leq(key);
    }

    const_iterator max_leq(const key_type& key) const override {
        return tree.max_leq(key);
    }
    
    iterator min_geq(const key_type& key) override {
        return tree.min_geq(key);
    }

    const_iterator min_geq(const key_type& key) const override {
        return tree.min_geq(key);
    }

    key_compare key_comp() const override {
        return tree.get_comparator();
    }
};
}