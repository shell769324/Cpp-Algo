#pragma once
#include "avl_tree.h"

namespace algo {
template<typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<std::pair<const Key, T> > >
class avl_tree_map {
private:
    using avl_tree_type = avl_tree<Key, std::pair<const Key, T>, pair_left_accessor<Key, T>, Compare, Allocator>;

public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using size_type = typename avl_tree_type::size_type;
    using difference_type = typename avl_tree_type::difference_type;
    using key_compare = typename avl_tree_type::key_compare;
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
     * @brief Construct an empty avl tree map with the default comparator
     */
    avl_tree_map() = default;

    /**
     * @brief Construct an empty avl tree map with a comparator and an optional allocator
     * 
     * @param comp the key comparator used by the map
     * @param allocator the allocator to construct and destroy key value pairs
     */
    explicit avl_tree_map(const Compare& comp, const Allocator& allocator = Allocator()) : tree(comp, allocator) { }

    /**
     * @brief Construct an empty avl tree map with an allocator
     * 
     * @param allocator the allocator to construct and destroy key value pairs
     */
    explicit avl_tree_map(const Allocator& allocator) : tree(Compare(), allocator) { }

    /**
     * @brief Construct a new avl tree map from a range with an optional comparator and an optional allocator
     * 
     * @tparam InputIt the type of the iterators that define the range
     * @param first iterator to the first element in the range
     * @param last iterator to one past the last element in the range
     * @param comp the key comparator used by the map
     * @param allocator the allocator to construct and destroy key value pairs
     */
    template<std::input_iterator InputIt>
    avl_tree_map(InputIt first, InputIt last, 
                 const Compare& comp = Compare(), 
                 const Allocator& allocator = Allocator())
        : tree(first, last, comp, allocator) { }
    
    /**
     * @brief Construct a new avl tree map from a range with an allocator
     * 
     * @tparam InputIt the type of the iterators that define the range
     * @param first iterator to the first element in the range
     * @param last iterator to one past the last element in the range
     * @param allocator the allocator to construct and destroy key value pairs
     */
    template<std::input_iterator InputIt>
    avl_tree_map(InputIt first, InputIt last, const Allocator& allocator)
        : tree(first, last, Compare(), allocator) { }

    /**
     * @brief Construct a copy of another avl tree map
     * 
     * Copy constructor
     * 
     * @param other the map to copy from
     */
    avl_tree_map(const avl_tree_map& other) = default;

    /**
     * @brief Construct a copy of another avl tree map and an allocator
     * 
     * @param other the map to copy from
     * @param allocator the allocator to construct and destroy key value pairs
     */
    avl_tree_map(const avl_tree_map& other, const Allocator& allocator)
        : tree(other.tree, allocator) { }

    /**
     * @brief Construct a copy of another avl tree map by move
     * 
     * Move constructor
     * 
     * @param other the map to move from
     */
    avl_tree_map(avl_tree_map&& other) = default;

    /**
     * @brief Construct a copy of another avl tree map and an allocator by move
     * 
     * @param other the map to move from
     * @param allocator the allocator to construct and destroy key value pairs
     */
    avl_tree_map(avl_tree_map&& other, const Allocator& allocator)
        : tree(std::move(other.tree), allocator) { }

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

    /**
     * @brief Get a copy of the associated allocator
     */
    allocator_type get_allocator() const noexcept {
        return tree.get_allocator();
    }

    /**
     * @brief Get the value given a key
     * 
     * If the key is not found, an exception is thrown
     * 
     * @param key the key associated with the value
     * @return a reference to value associated with the key
     */
    T& at(const key_type& key) {
        iterator it = tree.find(key);
        if (it == tree.end()) {
            throw std::out_of_range("Key not found");
        }
        return (*it).second;
    }

    /**
     * @brief Get the value given a key
     * 
     * If the key is not found, an exception is thrown
     * 
     * @param key the key associated with the value
     * @return a const reference to value associated with the key
     */
    const T& at(const key_type& key) const {
        const_iterator it = tree.find(key);
        if (it == tree.end()) {
            throw std::out_of_range("Key not found");
        }
        return (*it).second;
    }

    /**
     * @brief Get the value given a key
     * 
     * If the key is not found, a value is default constructed and inserted
     * 
     * @param key the key associated with the value
     * @return a reference to value associated with the key
     */
    T& operator[](const key_type& key) {
        std::pair<iterator, bool> emplace_result = tree.try_emplace(key, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple());
        return (*emplace_result.first).second;
    }
    
    /**
     * @brief Get the value given a key
     * 
     * If the key is not found, a value is default constructed. The
     * key will be moved and inserted along with the default value.
     * 
     * @param key the key associated with the value
     * @return a reference to value associated with the key
     */
    T& operator[](key_type&& key) {
        std::pair<iterator, bool> emplace_result = tree.try_emplace(key, std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple());
        return (*emplace_result.first).second;
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
     * @brief Test if this map has no elements
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
     * @brief Insert a key value pair by copy
     * 
     * @param value the value to insert
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this key already exists
     * 
     * The boolean is true if insertion succeeded, namely the key is not found, false otherwise
     */
    std::pair<iterator, bool> insert(const value_type& value) {
        return tree.try_emplace(value.first, value);
    }

    /**
     * @brief Insert a key value pair by move copy
     * 
     * @param value the value to insert
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this key already exists
     * 
     * The boolean is true if insertion succeeded, namely the key is not found, false otherwise
     */
    std::pair<iterator, bool> insert(value_type&& value) {
        return tree.try_emplace(value.first, std::move(value));
    }

    /**
     * @brief Insert a key value pair by move copy
     * 
     * @tparam P a type that can be converted to value_type
     * @param value the value used to construct the key value pair
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this key already exists
     * 
     * The boolean is true if insertion succeeded, namely the key is not found, false otherwise
     */
    template<typename P>
    std::pair<iterator, bool> insert(P&& value)
        requires std::constructible_from<value_type, P> {
        return tree.emplace(std::forward<P>(value));
    }

    /**
     * @brief Insert all key value pairs in a range
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
     * @brief In-place construct and insert a key value pair given the arguments
     *        to construct it
     * 
     * The key value pair is always constructed, regardless if the key already exists
     * 
     * @tparam the type of arguments to construct the key value pair
     * @param args the arguments to construct the key value pair
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this key already exists
     * 
     * The boolean is true if insertion succeeded, namely the key is not found, false otherwise
     */
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return tree.emplace(std::forward<Args>(args)...);
    }

    /**
     * @brief In-place construct and insert a key value pair given the key
     *        and the arguments to construct the value
     * 
     * The key value pair is constructed only if the key is not found
     * 
     * @tparam the type of arguments to construct the value
     * @param args the arguments to construct the value
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this key already exists
     * 
     * The boolean is true if insertion succeeded, namely the key is not found, false otherwise
     */
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(const key_type& key, Args&&... args) {
        return tree.try_emplace(key, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<Args>(args)...));
    }

    /**
     * @brief In-place construct and insert a key value pair given the key
     *        and the arguments to construct the value
     * 
     * The key will be moved to construct the pair
     * The key value pair is constructed only if the key is not found
     * 
     * @tparam the type of arguments to construct the value
     * @param args the arguments to construct the value
     * @return a pair of an iterator and a boolean
     * 
     * The iterator points at the inserted pair, or the existing one if this key already exists
     * 
     * The boolean is true if insertion succeeded, namely the key is not found, false otherwise
     */
    template <typename... Args>
    std::pair<iterator, bool> try_emplace(key_type&& key, Args&&... args) {
        return tree.try_emplace(std::move(key), std::piecewise_construct, std::forward_as_tuple(std::move(key)), std::forward_as_tuple(std::forward<Args>(args)...));
    }

    /**
     * @brief Remove the key value pair pointed by a given iterator
     * 
     * @param pos a iterator that points to the key value pair to remove
     * @return the iterator to the successor of the removed pair
     */
    iterator erase(iterator pos) {
        return tree.erase(pos);
    }

    /**
     * @brief Remove the key value pair pointed by a given iterator
     * 
     * @param pos a const iterator that points to the key value pair to remove
     * @return the iterator to the successor of the removed pair
     */
    iterator erase(const_iterator pos) {
        return tree.erase(pos);
    }

    /**
     * @brief Remove all key value pairs in a range
     * 
     * @param first the iterator to the first pair in the range
     * @param last the iterator to one past the last pair in the range
     * @return the iterator to the successor of the last removed pair
     */
    iterator erase(iterator first, iterator last) {
        return tree.erase(first, last);
    }
    
    /**
     * @brief Remove the value associated with a given key
     * 
     * @param key the key of the value to erase
     * @return true if removal happened (i.e. the key was found), false otherwise
     */
    bool erase(const key_type& key) {
        return tree.erase(key);
    }

    /**
     * @brief Swap content with another tree
     * 
     * @param other the tree to swap content with
     */
    void swap(avl_tree_map& other) noexcept(std::is_nothrow_swappable_v<Compare>) {
        tree.swap(other.tree);
    }

    /**
     * @brief Check equality of two avl tree maps
     * 
     * @param map1 the first avl tree map
     * @param map2 the second avl tree map
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const avl_tree_map& map1, const avl_tree_map& map2) requires equality_comparable<value_type> {
        return map1.tree == map2.tree;
    }

    /**
     * @brief Compare two avl tree maps
     * 
     * @param map1 the first avl tree map
     * @param map2 the second avl tree map
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const avl_tree_map& map1, const avl_tree_map& map2) requires less_comparable<value_type> {
        return map1.tree <=> map2.tree;
    }
    
    /**
     * @brief Get the iterator to a key value pair given its key
     * 
     * @param key the key to query
     * @return an iterator to the key value pair if the key is found
     *         an iterator equivalent to end() otherwise
     */
    iterator find(const key_type& key) {
        return tree.find(key);
    }

    /**
     * @brief Get the iterator to a key value pair given its key
     * 
     * @param key the key to query
     * @return a const iterator to the key value pair if the key is found
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator find(const key_type& key) const {
        return tree.find(key);
    }

    /**
     * @brief Test if a key exists
     * 
     * @param key the key to query
     * @return true if the key is found, false otherwise
     */
    bool contains(const key_type& key) const {
        return tree.find(key) != tree.end();
    }

    /**
     * @brief Get the iterator to a key value pair that has the smallest element greater 
     *        than a given key
     * 
     * @param key the key to query
     * @return an iterator to such key value pair if it exists
     *         an iterator equivalent to end() otherwise
     */
    iterator upper_bound(const key_type& key) {
        return tree.upper_bound(key);
    }

    /**
     * @brief Get the const iterator to a key value pair that has the smallest element greater 
     *        than a given key
     * 
     * @param key the key to query
     * @return a const iterator to such key value pair if it exists
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator upper_bound(const key_type& key) const {
        return tree.upper_bound(key);
    }

    /**
     * @brief Get the iterator to a key value pair that has the
     *        smallest key greater than or equal to a given key
     * 
     * @param key the key to query
     * @return an iterator to such key value pair if it exists
     *         an iterator equivalent to end() otherwise
     */
    iterator lower_bound(const key_type& key) {
        return tree.lower_bound(key);
    }

    /**
     * @brief Get the iterator to a key value pair that has the
     *        smallest key greater than or equal to a given key
     * 
     * @param key the key to query
     * @return a const iterator to such key value pair if it exists
     *         a const iterator equivalent to end() otherwise
     */
    const_iterator lower_bound(const key_type& key) const {
        return tree.lower_bound(key);
    }

    /**
     * @brief Compute the union of two avl tree maps
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the map
     *         It takes in two values and output true if the first value should be picked
     * @param map1 the first operand of the union operation
     * @param map2 the second operand of the union operation
     * @param resolver a conflict resolution function
     * @return the union of the avl tree maps 
     */
    template<typename Resolver=chooser<value_type> >
    friend avl_tree_map union_of(avl_tree_map map1, avl_tree_map map2, Resolver resolver=Resolver())
        requires is_resolver<value_type, Resolver> {
        map1.tree = union_of(std::move(map1.tree), std::move(map2.tree), resolver);
        return map1;
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree_map union_of(avl_tree_map map1, avl_tree_map map2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) requires is_resolver<value_type, Resolver>  {
        map1.tree = union_of(std::move(map1.tree), std::move(map2.tree), executor, resolver);
        return map1;
    }

    /**
     * @brief Compute the intersection of two avl tree maps
     * 
     * @tparam Resolver a function that resolves conflicts if the key also exists in the map
     *         It takes in two values and output true if the first value should be picked
     * @param map1 the first operand of the intersection operation
     * @param map2 the second operand of the intersection operation
     * @param resolver a conflict resolution function
     * @return the intersection of the avl tree maps 
     */
    template<typename Resolver=chooser<value_type> >
    friend avl_tree_map intersection_of(avl_tree_map map1, avl_tree_map map2, Resolver resolver=Resolver())
        requires is_resolver<value_type, Resolver> {
        map1.tree = intersection_of(std::move(map1.tree), std::move(map2.tree), resolver);
        return map1;
    }

    template<typename Resolver=chooser<value_type> >
    friend avl_tree_map intersection_of(avl_tree_map map1, avl_tree_map map2, thread_pool_executor& executor,
        Resolver resolver=Resolver()) requires is_resolver<value_type, Resolver> {
        map1.tree = intersection_of(std::move(map1.tree), std::move(map2.tree), executor, resolver);
        return map1;
    }

    /**
     * @brief Compute the difference of two avl tree maps
     * 
     * @param map1 the map to subtract from
     * @param map2 the map that subtracts
     * @return the difference of the avl tree maps 
     */
    friend avl_tree_map difference_of(avl_tree_map map1, avl_tree_map map2) {
        map1.tree = difference_of(std::move(map1.tree), std::move(map2.tree));
        return map1;
    }

    friend avl_tree_map difference_of(avl_tree_map map1, avl_tree_map map2, thread_pool_executor& executor) {
        map1.tree = difference_of(std::move(map1.tree), std::move(map2.tree), executor);
        return map1;
    }

    /**
     * @brief Get the key comparison object
     */
    key_compare key_comp() const {
        return tree.key_comp();
    }

    // For testing purpose
    bool __is_valid() const noexcept {
        return tree.__is_valid();
    }
};
}
