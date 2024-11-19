#pragma once

#include <iterator>
#include "src/deque/deque_constants.h"
#include "src/deque/deque_iterator.h"
#include "src/common.h"

namespace algo {

template<typename T, typename Allocator = std::allocator<T>>
    requires std::same_as<T, typename std::allocator_traits<Allocator>::value_type>
class deque {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = deque_iterator<T>::difference_type;
    using reference = T&;
    using const_reference = const T&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using iterator = deque_iterator<T>;
    using const_iterator = deque_iterator<const T>;
    using reverse_iterator = deque_iterator<value_type, true>;
    using const_reverse_iterator = deque_iterator<const value_type, true>;

private:
    using alloc_traits = std::allocator_traits<allocator_type>;
    using pointer_allocator_type = typename alloc_traits::rebind_alloc<T*>;
    using pointer_alloc_traits = typename alloc_traits::rebind_traits<T*>;

    size_type num_chunks;
    size_type num_elements;
    // Allocated chunks
    T** begin_chunk;
    T** end_chunk;

    // We need an empty chunk on both ends since we need to make sure both end()
    // and rend() don't dereference illegal memory;
    T** data;

    // Inclusive, pointing to the first element
    iterator begin_iterator;
    // Exclusive, pointing to the position to insert
    iterator end_iterator;

    allocator_type allocator;
    pointer_allocator_type pointer_allocator;

public:
    bool __is_valid() {
        size_type actual_num_elements = end_iterator - begin_iterator;
        if (actual_num_elements != num_elements || data > begin_chunk || begin_chunk > end_chunk 
            || end_chunk > data + num_chunks) {
            return false;
        }
        for (T** p = data - 1; p < begin_chunk; ++p) {
            if (*p != nullptr) {
                return false;
            }
        }
        for (T** p = end_chunk, **end = data + (num_chunks + 1); p < end; ++p) {
            if (*p != nullptr) {
                return false;
            }
        }
        for (T** p = begin_chunk; p < end_chunk; ++p) {
            if (*p == nullptr) {
                return false;
            }
        }
        return true;
    }

private:
    void fill_helper(size_type n, auto filler) {
        // Allow some room on both ends
        num_chunks = DEFAULT_NUM_CHUNKS + (n + CHUNK_SIZE<T> - 1) / CHUNK_SIZE<T>;
        num_elements = n;

        // We need to free both the outer memory and the inner memory in case of exception
        T** sentinel = pointer_alloc_traits::allocate(pointer_allocator, num_chunks + 2);
        std::unique_ptr<T*[], deleter<T*, pointer_allocator_type> > cleaner(
            sentinel, deleter<T*, pointer_allocator_type>(num_chunks + 2, pointer_allocator));
        data = sentinel + 1;
        begin_chunk = data + DEFAULT_NUM_CHUNKS / 2,
        end_chunk = begin_chunk;
        std::fill(sentinel, sentinel + num_chunks + 2, nullptr);
        try {
            for (size_type remain = n; remain > 0; ++end_chunk) {
                *end_chunk = alloc_traits::allocate(this -> allocator, CHUNK_SIZE<T>);
                filler(*end_chunk, std::min<size_type>(remain, CHUNK_SIZE<T>));
                size_type cap = std::min<size_type>(CHUNK_SIZE<T>, remain);
                remain -= cap;
            }
        } catch (...) {
            for (T** p = begin_chunk; p != end_chunk; ++p) {
                alloc_traits::deallocate(this -> allocator, *p, CHUNK_SIZE<T>);
            }
            throw;
        }
        begin_iterator = iterator(begin_chunk, *begin_chunk);
        end_iterator = begin_iterator + n;
        // Everything is allocated and initialized. We won't see exceptions anymore
        // Release unique_ptr ownership so our resources don't expire
        cleaner.release();
    }

public:
    /**
     * @brief Construct a deque with no elements
     */
    deque() : deque(allocator_type()) { }

    /**
     * @brief Construct an empty deque with an allocator
     * 
     * @param allocator the allocator to allocate and deallocate raw memory
     */
    explicit deque(const allocator_type& allocator) 
        : num_chunks(DEFAULT_NUM_CHUNKS),
          num_elements(0),
          allocator(allocator),
          pointer_allocator(allocator) {
        T** sentinel = pointer_alloc_traits::allocate(pointer_allocator, num_chunks + 2);
        data = sentinel + 1;
        std::fill(sentinel, sentinel + num_chunks + 2, nullptr);
        begin_chunk = data + num_chunks / 2;
        end_chunk = begin_chunk;
        begin_iterator = iterator(begin_chunk, *begin_chunk);
        end_iterator = begin_iterator;
    }

    /**
     * @brief Construct a deque with n default constructed elements
     * 
     * @param n the number of default constructed elements in the deque
     * @param allocator the allocator to allocate and deallocate raw memory
     */
    explicit deque(size_type n, const allocator_type& allocator = allocator_type()) 
        requires std::default_initializable<T> 
        : allocator(allocator),
          pointer_allocator(allocator) {
        auto filler = [this](T* p, size_type n) {
            uninitialized_default_construct(p, p + n, this -> allocator);
        };
        fill_helper(n, filler);
    }

    /**
     * @brief Construct a deque with n copies of a value
     * 
     * @param n the number of times to repeat the value in the deque
     * @param value the value to copy
     * @param allocator the allocator to allocate and deallocate raw memory
     */
    deque(size_type n, const_reference value, const allocator_type& allocator = allocator_type()) requires std::copy_constructible<T> 
        : allocator(allocator),
          pointer_allocator(allocator) {
        auto filler = [&value, this](T* p, size_type n) {
            uninitialized_fill(p, p + n, value, this -> allocator);
        };
        fill_helper(n, filler);
    }

    /**
     * @brief Construct a deque that contains values in a range
     * 
     * @tparam InputIt the type of the iterators that define the range
     * @param first the inclusive beginning of the range
     * @param last the exclusive end of the range
     * @param allocator the allocator to allocate and deallocate raw memory
     */
    template<std::input_iterator InputIt>
    deque(InputIt first, InputIt last, const allocator_type& allocator = allocator_type()) 
        requires std::copy_constructible<T> : deque(allocator) {
        insert(begin_iterator, first, last);
    }

private:
    void copy_constructor_helper(const deque& other) {
        // Prevent memory leak if copy or memory operations raise exceptions
        T** sentinel = pointer_alloc_traits::allocate(pointer_allocator, num_chunks + 2);
        std::unique_ptr<T*[], deleter<T*, pointer_allocator_type> > cleaner(
            sentinel, deleter<T*, pointer_allocator_type>(num_chunks + 2, pointer_allocator));
        data = sentinel + 1;
        std::fill(sentinel, sentinel + num_chunks + 2, nullptr);
        begin_chunk = data + (other.begin_chunk - other.data);
        end_chunk = data + (other.end_chunk - other.data);
        const_iterator other_begin = other.cbegin();
        const_iterator other_end = other.cend();
        T** curr = begin_chunk;
        try {
            for (; curr != end_chunk; ++curr) {
                *curr = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
            }
        } catch (...) {
            for (T** p = begin_chunk; p != curr; ++p) {
                alloc_traits::deallocate(allocator, *p, CHUNK_SIZE<T>);
            }
            throw;
        }
        begin_iterator = iterator(begin_chunk, *begin_chunk + (other_begin.inner_pointer - *other_begin.outer_pointer));
        end_iterator = uninitialized_copy(other_begin, other_end, begin_iterator, allocator);
        // Everything is allocated and initialized. We won't see exceptions anymore
        // Release unique_ptr ownership so our resources don't expire
        cleaner.release();
    }

public:
    /**
     * @brief Construct a copy of another deque
     * 
     * @param other the other deque to copy
     */
    deque(const deque& other) requires std::copy_constructible<T> 
        : num_chunks(other.num_chunks),
          num_elements(other.num_elements),
          allocator(other.allocator),
          pointer_allocator(other.pointer_allocator) {
        copy_constructor_helper(other);
    }

    /**
     * @brief Construct a copy of another deque with an allocator
     * 
     * @param other the other deque to copy
     * @param allocator the allocator to allocate and deallocate raw memory 
     */
    deque(const deque& other, const std::type_identity_t<Allocator>& allocator) requires std::copy_constructible<T> 
        : num_chunks(other.num_chunks),
          num_elements(other.num_elements),
          allocator(allocator),
          pointer_allocator(allocator) {
        copy_constructor_helper(other);
    }

    /**
     * @brief Construct a copy of another deque by move
     * 
     * The other deque shouldn't be used again but it is safe to destroy
     * 
     * @param other the other deque to move
     */
    deque(deque&& other) noexcept 
        : num_chunks(0),
          num_elements(0),
          begin_chunk(nullptr),
          end_chunk(nullptr),
          data(nullptr) {
        swap(other);
    }

    /**
     * @brief Construct a copy of another deque with an allocator by move
     * 
     * The other deque shouldn't be used again but it is safe to destroy
     * 
     * @param other the other deque to move
     * @param allocator the allocator to allocate and deallocate raw memory 
     */
    deque(deque&& other, const std::type_identity_t<Allocator>& allocator) noexcept 
        : num_chunks(0),
          num_elements(0),
          begin_chunk(nullptr),
          end_chunk(nullptr),
          data(nullptr),
          allocator(allocator),
          pointer_allocator(allocator) {
        std::swap(data, other.data);
        std::swap(num_chunks, other.num_chunks);
        std::swap(begin_iterator, other.begin_iterator);
        std::swap(end_iterator, other.end_iterator);
        std::swap(begin_chunk, other.begin_chunk);
        std::swap(end_chunk, other.end_chunk);
        std::swap(num_elements, other.num_elements);
    }

    /**
     * @brief Construct a deque object that contains the values in an initializer list
     * 
     * @param list the values to copy into the deque
     * @param allocator the allocator to allocate and deallocate raw memory
     */
    deque(std::initializer_list<value_type> list, const allocator_type& allocator = allocator_type()) 
        : deque(list.begin(), list.end(), allocator) {}

    /**
     * @brief Replaces this deque's contents with a copy of the contents of another deque
     * 
     * Assignment operator
     * 
     * @param other the deque to copy
     * @return a reference to self
     */
    deque& operator=(const deque& other) requires std::copy_constructible<T> {
        if (this == &other) {
            return *this;
        }

        if constexpr (std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_destructible_v<T>) {
            size_type total_capacity = (end_chunk - begin_chunk) * CHUNK_SIZE<T>;
            if (total_capacity >= other.num_elements) {
                destroy(begin_iterator, end_iterator, allocator);
                begin_iterator = iterator(begin_chunk, *begin_chunk) + (total_capacity - other.num_elements) / 2;
                end_iterator = uninitialized_copy(other.cbegin(), other.cend(), begin_iterator, allocator);
                num_elements = other.num_elements;
                allocator = other.allocator;
                pointer_allocator = other.pointer_allocator;
                return *this;
            }
        }
        
        deque tmp(other);
        swap(tmp);
        // tmp will be recycled because it goes out of scope
        return *this;
    }

    /**
     * @brief Replaces this deque's contents with the contents of another deque
     * 
     * The other deque shouldn't be used again but it is safe to destroy
     * 
     * Move assignment operator
     * 
     * @param other the deque to move
     * @return a reference to self
     */
    deque& operator=(deque&& other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief Destroy the deque
     */
    ~deque() {
        // It is possible data is just a nullptr. It can happen after
        // move constructor. In this case, there is no resource to free
        if (data == nullptr) {
            return;
        }
        destroy(begin_iterator, end_iterator, allocator);
        for (T** p = begin_chunk; p < end_chunk; ++p) {
            alloc_traits::deallocate(allocator, *p, CHUNK_SIZE<T>);
        }
        pointer_alloc_traits::deallocate(pointer_allocator, data - 1, num_chunks + 2);
    }

    /**
     * @brief Get a copy of the associated allocator
     */
    allocator_type get_allocator() {
        return allocator;
    }

    /**
     * @brief Get a reference to the element at a position
     * 
     * @param pos the position of the element
     * @return a reference to the element
     */
    reference operator[](size_type pos) {
        return const_cast<reference>(static_cast<const deque&>(*this)[pos]);
    }

    /**
     * @brief Get a const reference to the element at a position
     * 
     * @param pos the position of the element
     * @return a const reference to the element
     */
    const_reference operator[](size_type pos) const {
        iterator it = begin_iterator + pos;
        return *it;
    }

    /**
     * @brief Get a reference to the first element in the deque
     */
    reference front() const {
        return *begin_iterator;
    }

    /**
     * @brief Get a reference to the last element in the deque
     */
    reference back() const {
        return *(end_iterator - 1);
    }

    /**
     * @brief Get an iterator to the first element
     */
    iterator begin() noexcept {
        return begin_iterator;
    }

    /**
     * @brief Get a constant iterator to the first element
     */
    const_iterator begin() const noexcept {
        return const_iterator(begin_iterator);
    }

    /**
     * @brief Get a constant iterator to the first element
     */
    const_iterator cbegin() const noexcept {
        return const_iterator(begin_iterator);
    }

    /**
     * @brief Get a reverse iterator to the first element of the reversed deque
     */
    reverse_iterator rbegin() noexcept {
        reverse_iterator it = reverse_iterator(end_iterator);
        ++it;
        return it;
    }

    /**
     * @brief Get a constant reverse iterator to the first element of the reversed deque
     */
    const_reverse_iterator rbegin() const noexcept {
        const_reverse_iterator it = const_reverse_iterator(end_iterator);
        ++it;
        return it;
    }

    /**
     * @brief Get a constant reverse iterator to the first element of the reversed deque
     */
    const_iterator crbegin() const noexcept {
        return rbegin();
    }

    /**
     * @brief Get an iterator to the element following the last element
     */
    iterator end() noexcept {
        return end_iterator;
    }

    /**
     * @brief Get a const iterator to the element following the last element
     */
    const_iterator end() const noexcept {
        return const_iterator(end_iterator);
    }

    /**
     * @brief Get a const iterator to the element following the last element
     */
    const_iterator cend() const noexcept {
        return const_iterator(end_iterator);
    }

    /**
     * @brief Get a reverse iterator to the element following the last element of the reversed deque
     */
    reverse_iterator rend() noexcept {
        reverse_iterator it = reverse_iterator(begin_iterator);
        ++it;
        return it;
    }

    /**
     * @brief Get a const reverse iterator to the element following the last element of the reversed deque
     */
    const_reverse_iterator rend() const noexcept {
        const_reverse_iterator it = const_reverse_iterator(begin_iterator);
        ++it;
        return it;
    }

    /**
     * @brief Get a const reverse iterator to the element following the last element of the reversed deque
     */
    const_iterator crend() const noexcept {
        return rend();
    }

    /**
     * @brief Return true if this deque contains no elements
     */
    bool empty() const noexcept {
        return num_elements == 0;
    }

    /**
     * @brief Get the number of elements in this deque
     */
    size_type size() const noexcept {
        return num_elements;
    }

private:

    /**
     * @brief Resize outer array to a size and place active chunks at the center of
     *        the new array
     * 
     * No constructors/destructor of T are invoked. All references remain valid.
     * 
     * @pre begin_iterator_chunk - data < num_new_chunks
     * 
     * @param num_new_chunks new chunks needed to the left the active chunks
     * @param begin_iterator_chunk pointing to the chunk pointed to by begin iterator
     * @param end_iterator_chunk pointing to first chunk >= begin iterator chunk such that is holds no elements
     */
    void reallocate_end(size_type num_new_chunks, T** begin_iterator_chunk, T** end_iterator_chunk) {
        size_type active_chunks = end_iterator_chunk - begin_iterator_chunk + num_new_chunks;
        size_type new_num_chunks = active_chunks * 3;
        T** sentinel = pointer_alloc_traits::allocate(pointer_allocator, new_num_chunks + 2);
        std::unique_ptr<T*[], deleter<T*, pointer_allocator_type> > cleaner(
            sentinel, deleter<T*, pointer_allocator_type>(new_num_chunks + 2, pointer_allocator));
        T** new_data = sentinel + 1;
        std::fill_n(sentinel, new_num_chunks + 2, nullptr);
        /*
         * num_new_chunks = 3
         * active_chunks = 2 + 3 = 5
         * Before
         *      bic eic
         *       v v
         *   .***$$*.
         *    ^     ^
         *    bc    ec
         * After
         * .....*$$***....
         *      ^     ^
         *      bc    ec
         */
        T** new_begin_chunk = new_data + active_chunks;
        T** new_end_chunk = new_begin_chunk + active_chunks;
        T** missing_start = std::copy(begin_iterator_chunk, end_chunk, new_begin_chunk);
        std::ptrdiff_t needed = new_end_chunk - missing_start;
        std::ptrdiff_t num_free_chunks = begin_iterator_chunk - begin_chunk;
        if (needed <= num_free_chunks) {
            std::ptrdiff_t half = (num_free_chunks - needed) / 2;
            T** remain = begin_iterator_chunk - needed - half;
            end_chunk = std::copy(remain, begin_iterator_chunk, missing_start);
            begin_chunk = std::copy_backward(begin_chunk, remain, new_begin_chunk);
        } else {
            missing_start = std::copy(begin_chunk, begin_iterator_chunk, missing_start);
            T** p = missing_start;
            try {
                while (p < new_end_chunk) {
                    *p = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
                    p++;
                }
            } catch (...) {
                for (T** q = missing_start; q < p; q++) {
                    alloc_traits::deallocate(allocator, *q, CHUNK_SIZE<T>);
                }
                throw;
            }
            begin_chunk = new_begin_chunk;
            end_chunk = new_end_chunk;
        }
        // All memory allocation done. No more exception is possible
        cleaner.release();
        size_type old_num_chunks = num_chunks;
        num_chunks = new_num_chunks;
        begin_iterator.outer_pointer = new_begin_chunk;
        // Special case: if this deque contains no element, the chunk pointed to by
        // the old begin and end iterators may be reappropriated
        if (num_elements == 0) {
            begin_iterator.inner_pointer = *begin_iterator.outer_pointer;
        }
        end_iterator = begin_iterator + num_elements;
        // delete expression can't throw since delete operator can't throw and pointers have trivial destructor
        pointer_alloc_traits::deallocate(pointer_allocator, data - 1, old_num_chunks + 2);
        data = new_data;
    }

    void center(std::ptrdiff_t left, std::ptrdiff_t right) {
        std::ptrdiff_t donation = (right - left) / 2;
        if (donation >= 0) {
            // Too many on the right
            begin_chunk -= donation;
            std::swap_ranges(end_chunk - donation, end_chunk, begin_chunk);
            end_chunk -= donation;
        } else {
            donation = -donation;
            // Too many on the left
            end_chunk = std::swap_ranges(begin_chunk, begin_chunk + donation, end_chunk);
            begin_chunk += donation;
        }
    }

    void rearrange_end(size_type num_new_chunks, T** begin_iterator_chunk, T** end_iterator_chunk) {
        size_type active_chunks = end_iterator_chunk - begin_iterator_chunk + num_new_chunks;
        T** new_begin_chunk = data + (num_chunks - active_chunks) / 2;
        T** new_end_iterator_chunk = std::swap_ranges(begin_iterator_chunk, end_iterator_chunk, new_begin_chunk);
        begin_iterator.outer_pointer = new_begin_chunk;
        if (num_elements == 0) {
            begin_iterator.inner_pointer = *begin_iterator.outer_pointer;
        }
        end_iterator = begin_iterator + num_elements;
        if (begin_chunk > new_begin_chunk) {
            /*         bic ec
             *          v  v
             * .....****12*
             *      ^     ^
             *      bc   eic
             *    bic
             *     v
             * ....12***.**
             *       ^
             *      eic
             * ....12****.. coalesce
             */
            T** fill_pos;
            if (begin_chunk <= new_end_iterator_chunk) {
                fill_pos = begin_iterator_chunk;
            } else {
                fill_pos = std::swap_ranges(begin_chunk, begin_iterator_chunk, new_end_iterator_chunk);
            }
            T** src_begin = end_iterator_chunk - (begin_chunk <= new_end_iterator_chunk ? new_end_iterator_chunk - begin_chunk : 0);
            end_chunk = std::swap_ranges(src_begin, end_chunk, fill_pos);
            begin_chunk = new_begin_chunk;
            T** new_end_chunk = new_begin_chunk + active_chunks;
            bool balanced = end_chunk <= new_end_chunk + 1;
            // We don't need to free the chunks if we encounter an exception
            // Update begin_chunk on the fly 
            while (end_chunk < new_end_chunk) {
                *end_chunk = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
                ++end_chunk;
            }
            // If we were short of chunks or we have at most one extra, we are already balanced
            if (balanced) {
                return;
            }
        }
        /*
         * Assume a single contiguous of free chunks on the right
         * num_new_chunks = 2
         * ...******12.
         * ...*12*****.
         * ..**12****..
         */
        std::ptrdiff_t left = new_begin_chunk - begin_chunk;
        std::ptrdiff_t right = end_chunk - new_end_iterator_chunk - num_new_chunks;
        center(left, right);
    }

    /**
     * @brief Remap or rearrange existing chucnks such that we have a specified
     *        number of allocated chunks available after the current active chunks
     * 
     * No constructor, destructor or assignment operators of T are called
     * 
     * @param num_new_chunks new chunks needed to the right the active chunks
     */
    void make_room_end(size_type num_new_chunks) {
        // We shouldn't always triple the size since there could be very few active chunks
        // We just need to move them to the center
        T** begin_iterator_chunk = begin_iterator.outer_pointer;
        T** end_iterator_chunk = num_elements == 0 ? begin_iterator_chunk
            : (end_iterator - 1).outer_pointer + 1;

        size_type active_chunks = end_iterator_chunk - begin_iterator_chunk + 
                                   num_new_chunks;
        if (active_chunks <= num_chunks / 3) {
            rearrange_end(num_new_chunks, begin_iterator_chunk, end_iterator_chunk);
        } else {
            reallocate_end(num_new_chunks, begin_iterator_chunk, end_iterator_chunk);
        }
    }

    void allocate_tail_space() {
        if (end_iterator.outer_pointer == data + num_chunks) {
            make_room_end(1);
        } else if (*end_iterator.outer_pointer == nullptr) {
            T* memory = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
            ++end_chunk;
            std::ptrdiff_t diff = *end_iterator.outer_pointer - end_iterator.inner_pointer;
            *end_iterator.outer_pointer = memory;
            if (*begin_iterator.outer_pointer == memory) {
                begin_iterator.inner_pointer = memory + diff;
            }
            end_iterator.inner_pointer = memory + diff;
        }
    }

public:
    /**
     * @brief Appends the given element value to the end of the deque by copy
     * 
     * @param value the element to append
     */
    void push_back(const_reference value) requires std::copy_constructible<T> {
        allocate_tail_space();
        alloc_traits::construct(allocator, end_iterator.inner_pointer, value);
        ++end_iterator;
        ++num_elements;
    }

    /**
     * @brief Appends the given element value to the end of the deque by move
     * 
     * @param value the element to append
     */
    void push_back(value_type&& value) requires std::move_constructible<T> {
        allocate_tail_space();
        alloc_traits::construct(allocator, end_iterator.inner_pointer, std::move(value));
        ++end_iterator;
        ++num_elements;
    }

    /**
     * @brief Construct and append an element to the end of the deque in-place
     * 
     * @param args the arguments used to construct the elements in place
     */
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        allocate_tail_space();
        alloc_traits::construct(allocator, end_iterator.inner_pointer, std::forward<Args>(args)...);
        reference res = *end_iterator.inner_pointer;
        ++end_iterator;
        ++num_elements;
        return res;
    }

    /**
     * @brief Removes the last element of the container.
     */
    void pop_back() noexcept(std::is_nothrow_destructible_v<T>) {
        iterator new_end_iterator = end_iterator - 1;
        alloc_traits::destroy(allocator, new_end_iterator.inner_pointer);
        end_iterator = new_end_iterator;
        --num_elements;
    }

private:

    /**
     * @brief Resize outer array to a size and place active chunks at the center of
     *        the new array
     * 
     * No constructors/destructor of T are invoked. All references remain valid.
     * 
     * @pre begin_iterator_chunk - data < num_new_chunks
     * 
     * @param num_new_chunks new chunks needed to the left the active chunks
     * @param begin_iterator_chunk pointing to the chunk pointed to by begin iterator
     * @param end_iterator_chunk pointing to first chunk >= begin iterator chunk which holds no elements
     */
    void reallocate_begin(size_type num_new_chunks, T** begin_iterator_chunk, T** end_iterator_chunk) {
        size_type active_chunks = end_iterator_chunk - begin_iterator_chunk + num_new_chunks;
        size_type new_num_chunks = active_chunks * 3;
        T** sentinel = pointer_alloc_traits::allocate(pointer_allocator, new_num_chunks + 2);
        std::unique_ptr<T*[], deleter<T*, pointer_allocator_type> > cleaner(
            sentinel, deleter<T*, pointer_allocator_type>(new_num_chunks + 2, pointer_allocator));
        T** new_data = sentinel + 1;
        std::fill_n(sentinel, new_num_chunks + 2, nullptr);
        /*
         * num_new_chunks = 3
         * active_chunks = 2 + 3 = 5
         * Before
         *       bic eic
         *         v v
         *       .*$$***.
         *        ^     ^
         *        bc    ec
         * Then
         * .......*$$.....
         *        ^
         *        me
         * After
         * .....***$$*....
         *      ^     ^
         *      bc    ec
         */
        T** new_begin_chunk = new_data + active_chunks;
        T** new_end_chunk = new_data + active_chunks * 2;
        T** missing_end = std::copy_backward(begin_chunk, end_iterator_chunk, new_end_chunk);
        std::ptrdiff_t needed = missing_end - new_begin_chunk;
        std::ptrdiff_t num_free_chunks = end_chunk - end_iterator_chunk;
        if (needed <= num_free_chunks) {
            std::ptrdiff_t half = (num_free_chunks - needed) / 2;
            T** remain = end_iterator_chunk + needed + half;
            begin_chunk = std::copy_backward(end_iterator_chunk, remain, missing_end);
            end_chunk = std::copy(remain, end_chunk, new_end_chunk);
        } else {
            T** fill_end = std::copy_backward(end_iterator_chunk, end_chunk, missing_end);
            T** p = new_begin_chunk;
            try {
                while (p < fill_end) {
                    *p = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
                    p++;
                }
            } catch (...) {
                for (T** q = new_begin_chunk; q < p; q++) {
                    alloc_traits::deallocate(allocator, *q, CHUNK_SIZE<T>);
                }
                throw;
            }
            begin_chunk = new_begin_chunk;
            end_chunk = new_end_chunk;
        }
        // All memory allocation done. No more exception is possible
        cleaner.release();
        size_type old_num_chunks = num_chunks;
        num_chunks = new_num_chunks;
        begin_iterator.outer_pointer = new_begin_chunk + num_new_chunks;
        // Special case: if this deque contains no element, the chunk pointed to by
        // the old begin and end iterators may be reappropriated
        if (num_elements == 0) {
            begin_iterator.inner_pointer = *begin_iterator.outer_pointer;
        }
        end_iterator = begin_iterator + num_elements;
        // delete expression can't throw since delete operator can't throw and pointers have trivial destructor
        pointer_alloc_traits::deallocate(pointer_allocator, data - 1, old_num_chunks + 2);
        data = new_data;
    }

    /**
     * @brief Rearrange chunks in array to a size so that active chunks are at the center of
     *        the new array
     * 
     * No constructors/destructor of T are invoked. All references remain valid.
     * 
     * @pre end_iterator_chunk - begin_iterator_chunk + num_new_chunks <= num_chunks / 3
     * @pre end_iterator_chunk - data <= num_chunks / 3
     * 
     * @param num_new_chunks new chunks needed to the left the active chunks
     * @param begin_iterator_chunk pointing to the chunk pointed to by begin iterator
     * @param end_iterator_chunk pointing to first chunk >= begin iterator chunk such that is holds no elements
     */
    void rearrange_begin(size_type num_new_chunks, T** begin_iterator_chunk, T** end_iterator_chunk) {
        size_type active_chunks = end_iterator_chunk - begin_iterator_chunk + num_new_chunks;
        T** new_begin_chunk = data + (num_chunks - active_chunks) / 2;
        T** new_begin_iterator_chunk = new_begin_chunk + num_new_chunks;
        T** new_end_iterator_chunk = std::swap_ranges(begin_iterator_chunk, end_iterator_chunk, new_begin_iterator_chunk);
        begin_iterator.outer_pointer = new_begin_iterator_chunk;
        if (num_elements == 0) {
            begin_iterator.inner_pointer = *begin_iterator.outer_pointer;
        }
        end_iterator = begin_iterator + num_elements;
        if (end_chunk < new_end_iterator_chunk) {
            /*
             * num_new_chunks = 2
             * Before
             *  bic    ec
             *   v     v
             *  *12****.....
             *  ^  ^
             * bc eic
             * Move begin/end iterator chunks
             *       bic
             *        v
             *  **.***12....
             *          ^
             *         eic
             * After coalesce
             *  ..****12....
             */
            T** fill_pos;
            if (end_chunk >= new_begin_iterator_chunk) {
                fill_pos = end_iterator_chunk;
            } else {
                fill_pos = std::swap_ranges(
                    std::reverse_iterator<T**>(end_chunk - 1),
                    std::reverse_iterator<T**>(end_iterator_chunk - 1), 
                    std::reverse_iterator<T**>(new_begin_iterator_chunk - 1)).base() + 1;
            }
            T** src_end = begin_iterator_chunk + (end_chunk >= new_begin_iterator_chunk ? end_chunk - new_begin_iterator_chunk : 0);
            begin_chunk = std::swap_ranges(
                std::reverse_iterator<T**>(src_end - 1),
                std::reverse_iterator<T**>(begin_chunk - 1), 
                std::reverse_iterator<T**>(fill_pos)).base() + 1;
            end_chunk = new_end_iterator_chunk;
            bool balanced = begin_chunk >= new_begin_chunk - 1;
            T** p = begin_chunk - 1;
            // We don't need to free the chunks if we encounter an exception
            // Update begin_chunk on the fly. The data structure will stay in a valid state.
            while (p >= new_begin_chunk) {
                *p = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
                begin_chunk--;
                p--;
            }
            // If we were short of chunks or we have at most one extra, we are already balanced
            if (balanced) {
                return;
            }
        }

        /* num_new_chunks = 2
         *      .12******...
         * .*****12*...
         * ..****12**..
         * Single contiguous of free chunks on the left
         */
        std::ptrdiff_t left = new_begin_iterator_chunk - begin_chunk - num_new_chunks;
        std::ptrdiff_t right = end_chunk - new_end_iterator_chunk;
        center(left, right);
    }

    /**
     * @brief Remap or rearrange existing chucnks such that we have a specified
     *        number of allocated chunks available before the current active chunks
     * 
     * @param num_new_chunks new chunks needed to the left the active chunks
     */
    void make_room_begin(size_type num_new_chunks) {
        // We shouldn't always triple the size since there could be very few active chunks
        // We just need to move them to the center
        T** begin_iterator_chunk = begin_iterator.outer_pointer;
        T** end_iterator_chunk = num_elements == 0 ? begin_iterator_chunk
            : (end_iterator - 1).outer_pointer + 1;

        size_type active_chunks = end_iterator_chunk - begin_iterator_chunk + 
                                    num_new_chunks;
        if (active_chunks <= num_chunks / 3) {
            rearrange_begin(num_new_chunks, begin_iterator_chunk, end_iterator_chunk);
        } else {
            reallocate_begin(num_new_chunks, begin_iterator_chunk, end_iterator_chunk);
        }
    }

    iterator allocate_front_space() {
        if (begin_iterator.outer_pointer == data && begin_iterator.inner_pointer == *begin_iterator.outer_pointer) {
            make_room_begin(1);
            return begin_iterator - 1;
        }
        
        iterator prior = begin_iterator - 1;
        if (*prior.outer_pointer == nullptr) {
            T* memory = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
            begin_chunk--;
            std::ptrdiff_t diff = prior.inner_pointer - *prior.outer_pointer;
            *prior.outer_pointer = memory;
            if (*end_iterator.outer_pointer == memory) {
                end_iterator.inner_pointer = memory + diff;
            }
            prior.inner_pointer = memory + diff;
        }
        return prior;
    }
    
public:
    /**
     * @brief Prepend the given element value to the beginning of the deque by copy
     * 
     * @param value the element to append
     */
    void push_front(const_reference value) requires std::copy_constructible<T> {
        iterator prior = allocate_front_space();
        alloc_traits::construct(allocator, prior.inner_pointer, value);
        begin_iterator = prior;
        ++num_elements;
    }

    /**
     * @brief Prepend the given element value to the beginning of the deque by move
     * 
     * @param value the element to append
     */
    void push_front(value_type&& value) requires std::move_constructible<T> {
        iterator prior = allocate_front_space();
        alloc_traits::construct(allocator, prior.inner_pointer, std::move(value));
        begin_iterator = prior;
        ++num_elements;
    }

    /**
     * @brief Construct and prepend an element to the beginning of the deque in-place
     * 
     * @param args the arguments used to construct the elements in place
     */
    template<typename... Args>
    reference emplace_front(Args&&... args) {
        iterator prior = allocate_front_space();
        alloc_traits::construct(allocator, prior.inner_pointer, std::forward<Args>(args)...);
        begin_iterator = prior;
        ++num_elements;
        return *begin_iterator.inner_pointer;
    }

    void pop_front() {
        alloc_traits::destroy(allocator, begin_iterator.inner_pointer);
        ++begin_iterator;
        --num_elements;
    }

private:
    /**
     * @brief Reserve an unoccupied range of size amount before pos
     *        The begin of range could be partially uninitialized. 
     * 
     * @param pos the position to insert the elements. Existing elements on or after this pos
     *            after shifted right
     * @param amount the number elements to insert
     * @param uninitialized_begin the begin of the range to insert with constructor
     * @param initialized_begin the begin of the range to insert with assignment operator
     */
    void insert_shift_begin(iterator pos, size_type space, 
                            iterator& uninitialized_begin, iterator& initialized_begin) {
        difference_type amount = space;
        difference_type offset = pos - begin_iterator;
        difference_type remain = begin_iterator - iterator(data, data[0]);
        if (remain < amount) {
            difference_type ghost_begin = remain - amount;
            // Round down
            std::ptrdiff_t ghost_begin_chunk = ghost_begin / CHUNK_SIZE<T> - (ghost_begin % CHUNK_SIZE<T> < 0);
            make_room_begin(begin_iterator.outer_pointer - data - ghost_begin_chunk);
        } else {
            T** fill_start = (begin_iterator - amount).outer_pointer;
            // If we already have all chunks allocated, we don't need to allocate any more
            if (fill_start < begin_chunk) {
                T** p = fill_start;
                try {
                    for (; p < begin_chunk; p++) {
                        *p = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
                    }
                } catch (...) {
                    for (T** q = fill_start; q < p; q++) {
                        alloc_traits::deallocate(allocator, *q, CHUNK_SIZE<T>);
                    }
                    throw;
                }
                begin_chunk = fill_start;
            }
        }
        pos = begin_iterator + offset;
        iterator new_begin_iterator = begin_iterator - amount;
        // This is also the new end of the moved prefix
        uninitialized_begin = new_begin_iterator + offset;
        // If we are prepending, there is nothing to move.
        if (offset == 0) {
            initialized_begin = uninitialized_begin + amount;
            begin_iterator = new_begin_iterator;
            return;
        }
        /*
         * number of new chunks = 4
         * Before   
         *    bc  pos  eic
         *     v   v   v
         * ....*1234567*.....
         *      ^       ^
         *     bic      ec
         * Then
         * nbic    pos eic
         *  v      v   v
         * .****1234567*....
         *  ^   ^       ^
         * nbc bic      ec
         * After
         *    bc  pos eic
         *     v   v   v
         * .123uiii4567*....
         *  ^   ^       ^
         * nbc bic     ec
         */
        iterator uninitialized_src_end;
        if (uninitialized_begin <= begin_iterator) {
            // Whole prefix will be copied to an uninitialized range
            uninitialized_src_end = pos;
        } else {
            // [begin_iterator, uninitialized_begin) are already initialized
            // The portion before that is still uninitialized
            uninitialized_src_end = pos - (uninitialized_begin - begin_iterator);
        }
        iterator cont;
        // If we can move, we move. The value must be copy constructible when we reach this method, 
        // so copy should always bs a valid backup
        cont = try_uninitialized_move(begin_iterator, uninitialized_src_end, new_begin_iterator, allocator);
        try_move(uninitialized_src_end, pos, cont);

        if (uninitialized_begin <= begin_iterator) {
            initialized_begin = begin_iterator;
        } else {
            initialized_begin = uninitialized_begin;
        }
        begin_iterator = new_begin_iterator;
    }

    void insert_shift_end(iterator pos, size_type space,
                          iterator& initialized_begin, iterator& uninitialized_begin) {
        difference_type amount = space;
        difference_type offset = end_iterator - pos;
        difference_type remain = iterator(data + num_chunks, data[num_chunks]) - end_iterator;
        difference_type ghost_rbegin = end_iterator - iterator(data, *data) + amount - 1;
        T** new_end_chunk = data + ghost_rbegin / CHUNK_SIZE<T> + 1;
        if (remain < amount) {
            T** end_iterator_chunk = num_elements == 0 ? begin_iterator.outer_pointer
                                                       : (end_iterator - 1).outer_pointer + 1;
            make_room_end(new_end_chunk - end_iterator_chunk);
        } else {
            T** fill_end = (end_iterator + (amount - 1)).outer_pointer + 1;
            // If we already have all chunks allocated, we don't need to allocate any more
            if (end_chunk < fill_end) {
                T** p = end_chunk;
                try {
                    for (; p < fill_end; p++) {
                        *p = alloc_traits::allocate(allocator, CHUNK_SIZE<T>);
                    }
                } catch (...) {
                    for (T** q = end_chunk; q < p; q++) {
                        alloc_traits::deallocate(allocator, *q, CHUNK_SIZE<T>);
                    }
                    throw;
                }
                end_chunk = fill_end;
                // If end_iterator points to beginning of an unallocated end iterator chunk
                // update its inner pointer since the memory has been allocated
                if (end_iterator.inner_pointer == nullptr) {
                    end_iterator.inner_pointer = *end_iterator.outer_pointer;
                    // If the deque is empty, we need to update begin_iterator as well
                    if (num_elements == 0) {
                        begin_iterator == end_iterator;
                    }
                }
            }
        }
        pos = end_iterator - offset;
        initialized_begin = pos;
        iterator new_end_iterator = end_iterator + amount;
        // If we are appending, there is nothing to move. 
        if (offset == 0) {
            uninitialized_begin = pos;
            end_iterator = new_end_iterator;
            return;
        }
        /*
         * number of new chunks = 3
         * Before   
         *    bc  pos  eic
         *     v   v   v
         * ....*1234567*.....
         *      ^       ^
         *     bic      ec
         * Then
         *        pos eic
         *         v   v
         * ....*1234567***...
         *      ^         ^
         *     bic        ec
         * After
         *        pos eic
         *         v   v
         * ....*123iii4567...
         *      ^         ^
         *     bic        ec
         */
        iterator moved_dest_begin = new_end_iterator - offset;
        iterator uninitialized_src_begin;
        if (moved_dest_begin >= end_iterator) {
            // Whole suffix will be copied to an uninitialized range
            uninitialized_src_begin = pos;
        } else {
            uninitialized_src_begin = pos + (end_iterator - moved_dest_begin);
        }
        // If we can move, we move. The value must be copy constructible when we reach this method, 
        // so copy should always bs a valid backup
        try_uninitialized_move(
            reverse_iterator(end_iterator - 1),
            reverse_iterator(uninitialized_src_begin - 1), 
            reverse_iterator(new_end_iterator - 1),
            allocator
        );
        difference_type initialized_count = uninitialized_src_begin - pos;
        if constexpr (std::is_move_assignable_v<T>) {
            std::move_backward(pos, uninitialized_src_begin, moved_dest_begin + initialized_count);
        } else {
            std::copy_backward(pos, uninitialized_src_begin, moved_dest_begin + initialized_count);
        }
        if (moved_dest_begin <= end_iterator) {
            uninitialized_begin = pos + amount;
        } else {
            uninitialized_begin = end_iterator;
        }
        end_iterator = new_end_iterator;
    }

    template<typename... Args>
    iterator insert_single_helper(const_iterator pos, Args&&... args) {
        // cpp20 standard 22.3.8.4.1 states that insertion at either ends must be exception-safe
        if (pos == begin_iterator) {
            emplace_front(std::forward<Args>(args)...);
            return begin_iterator;
        }
        if (pos == end_iterator) {
            emplace_back(std::forward<Args>(args)...);
            return end_iterator - 1;
        }
        unsigned long offset = pos - begin_iterator;
        iterator uninitialized_begin, initialized_begin;
        // cpp20 standard 22.3.8.4.3 states that the time complexity must be the lesser of the
        // distance from pos to either ends
        if (offset * 2 <= num_elements) {
            insert_shift_begin(iterator(pos), 1, uninitialized_begin, initialized_begin);
        } else {
            insert_shift_end(iterator(pos), 1, initialized_begin, uninitialized_begin);
        }
        *initialized_begin = T(std::forward<Args>(args)...);
        ++num_elements;
        return initialized_begin;
    }

public:
    /**
     * @brief Inserts a copy of value before a position
     * 
     * @param pos the location to insert the value
     * @param value the value to copy
     * @return iterator to the inserted value
     */
    iterator insert(const_iterator pos, const_reference value)
        requires std::copy_constructible<T> && std::is_copy_assignable_v<T> {
        return insert_single_helper(pos, value);
    }

    /**
     * @brief Inserts a value before a position by move
     * 
     * @param pos the location to insert the value
     * @param value the value to move
     * @return iterator to the inserted value
     */
    iterator insert(const_iterator pos, value_type&& value)
        requires std::move_constructible<T> && std::is_copy_assignable_v<T> {
        return insert_single_helper(pos, std::move(value));
    }
    
    /**
     * @brief In-place construct an element and insert it before a position
     * 
     * @param pos the location to insert the value
     * @param args the arguments to construct the value in-place
     * @return iterator to the inserted value
     */
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
        requires std::copy_constructible<T> && std::is_copy_assignable_v<T> {
        return insert_single_helper(pos, std::forward<Args>(args)...);
    }

    /**
     * @brief Insert the same copy of the value a specific number of times before a position
     * 
     * @param pos the location to insert the value copies 
     * @param count the number of times to insert the value copies
     * @param value the value to insert
     * @return iterator to the first inserted value
     */
    iterator insert(const_iterator pos, size_type count, const_reference value)
        requires std::copy_constructible<T> && std::is_copy_assignable_v<T> {
        if (count == 1) {
            return insert(pos, value);
        }
        unsigned long offset = pos - begin_iterator;
        iterator uninitialized_begin, initialized_begin;
        // cpp20 standard 22.3.8.4.3 states that the time complexity must be the lesser of the
        // distance from pos to either ends
        if (offset * 2 <= num_elements) {
            insert_shift_begin(iterator(pos), count, uninitialized_begin, initialized_begin);
            uninitialized_fill(uninitialized_begin, initialized_begin, value, allocator);
            std::fill_n(initialized_begin, count - (initialized_begin - uninitialized_begin), value);
        } else {
            insert_shift_end(iterator(pos), count, initialized_begin, uninitialized_begin);
            std::fill(initialized_begin, uninitialized_begin, value);
            uninitialized_fill(uninitialized_begin, uninitialized_begin + count - (uninitialized_begin - initialized_begin), value, allocator);
        }
        num_elements += count;
        return begin_iterator + offset;
    }

    /**
     * @brief Insert all elements in a range before a position
     * 
     * The iterators defining the range must support multiple passes
     * 
     * @param pos the location to insert the value copies 
     * @param first the beginning of the range
     * @param last the end of the range
     * @return iterator to the first inserted value
     */
    template<std::forward_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last) 
        requires std::copy_constructible<T> && std::is_copy_assignable_v<T> {
        if (first == last) {
            return iterator(pos);
        }
        difference_type count = last - first;
        if (count == 1) {
            return insert(pos, *first);
        }
        unsigned long offset = pos - begin_iterator;
        iterator uninitialized_begin, initialized_begin;
        if (offset * 2 <= num_elements) {
            insert_shift_begin(iterator(pos), count, uninitialized_begin, initialized_begin);
            difference_type uninitialized_count = initialized_begin - uninitialized_begin;
            uninitialized_copy(first, first + uninitialized_count, uninitialized_begin, allocator);
            first = std::next(first, uninitialized_count);
            std::copy_n(first, count - uninitialized_count, initialized_begin);
        } else {
            insert_shift_end(iterator(pos), count, initialized_begin, uninitialized_begin);
            difference_type initialized_count = uninitialized_begin - initialized_begin;
            std::copy_n(first, initialized_count, initialized_begin);
            first = std::next(first, initialized_count);
            uninitialized_copy(first, first + (count - initialized_count), uninitialized_begin, allocator);
        }
        num_elements += count;
        return begin_iterator + offset;
    }

    /**
     * @brief Insert all elements in a range before a position
     * 
     * The iterators defining the range only supports a single pass
     * 
     * @param pos the location to insert the value copies 
     * @param first the beginning of the range
     * @param last the end of the range
     * @return iterator to the first inserted value
     */
    template<std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
        requires std::copy_constructible<T> && std::is_copy_assignable_v<T> && (!std::forward_iterator<InputIt>) {
        if (first == last) {
            return iterator(pos);
        }
        deque storage(allocator);
        for (InputIt& it = first; it != last; ++it) {
            storage.push_back(*it);
        }
        return insert(pos, storage.begin(), storage.end());
    }

private:
    void erase_shift_begin(iterator first, iterator last) requires std::is_copy_assignable_v<T> {
        if constexpr(std::is_move_assignable_v<T>) {
            std::move_backward(begin_iterator, first, last);
        } else {
            std::copy_backward(begin_iterator, first, last);
        }
        iterator new_begin_iterator = begin_iterator + (last - first);
        destroy(begin_iterator, new_begin_iterator, allocator);
        begin_iterator = new_begin_iterator;
    }

    void erase_shift_end(iterator first, iterator last) {
        try_move(last, end_iterator, first);
        reverse_iterator new_end_iterator = reverse_iterator(end_iterator - 1) + (last - first);
        destroy(reverse_iterator(end_iterator - 1), new_end_iterator, allocator);
        end_iterator = new_end_iterator;
        ++end_iterator;
    }

public:
    /**
     * @brief Erase the value at a position
     * 
     * @param pos the position of the value to erase
     * @return iterator to the value after the erased value
     */
    iterator erase(const_iterator pos) {
        if (pos == begin_iterator) {
            pop_front();
            return begin_iterator;
        }
        if (end_iterator - iterator(pos) == 1) {
            pop_back();
            return end_iterator;
        }
        difference_type offset = pos - begin_iterator;
        if ((size_type) offset * 2 <= num_elements) {
            erase_shift_begin(iterator(pos), iterator(pos + 1));
        } else {
            erase_shift_end(iterator(pos), iterator(pos + 1));
        }
        --num_elements;
        return begin_iterator + offset;
    }

    /**
     * @brief Erase the values in a range
     * 
     * @param first the inclusive beginning of the range to erase
     * @param last the exclusive end fo the range to erase
     * @return iterator to the value after the last erased value
     */
    iterator erase(const_iterator first, const_iterator last) {
        iterator non_const_first = iterator(first);
        iterator non_const_last = iterator(last);
        difference_type before = non_const_first - begin_iterator;
        difference_type after = end_iterator - non_const_last;
        difference_type count = non_const_last - non_const_first;
        if (before <= after) {
            erase_shift_begin(non_const_first, non_const_last);
        } else {
            erase_shift_end(non_const_first, non_const_last);
        }
        num_elements -= count;
        return begin_iterator + before;
    }

    /**
     * @brief Remove all elements from this deque
     */
    void clear() noexcept(std::is_nothrow_destructible_v<T>) {
        destroy(begin_iterator, end_iterator, allocator);
        end_iterator = begin_iterator;
        num_elements = 0;
    }

    void swap(deque& other) noexcept {
        std::swap(data, other.data);
        std::swap(num_chunks, other.num_chunks);
        std::swap(begin_iterator, other.begin_iterator);
        std::swap(end_iterator, other.end_iterator);
        std::swap(begin_chunk, other.begin_chunk);
        std::swap(end_chunk, other.end_chunk);
        std::swap(num_elements, other.num_elements);
        std::swap(allocator, other.allocator);
        std::swap(pointer_allocator, other.pointer_allocator);
    }

    /**
     * @brief Check equality of two deques
     * 
     * @param deque1 the first deque
     * @param deque2 the second deque
     * @return true if their contents are equal, false otherwise
     */
    friend bool operator==(const deque& deque1, const deque& deque2) noexcept requires equality_comparable<value_type> {
        return container_equals(deque1, deque2);
    }

    /**
     * @brief Compare two deques
     * 
     * @param deque1 the first deque
     * @param deque2 the second deque
     * @return a strong ordering comparison result
     */
    friend std::strong_ordering operator<=>(const deque& deque1, const deque& deque2) noexcept requires less_comparable<value_type> {
        return container_three_way_comparison(deque1, deque2);
    }

    size_type __get_num_chunks() {
        return num_chunks;
    }

    size_type __front_capacity() {
        if (*begin_iterator.outer_pointer == nullptr) {
            return 0;
        }
        return std::distance(*begin_iterator.outer_pointer, begin_iterator.inner_pointer);
    }

    size_type __front_ghost_capacity() {
        return __front_capacity() + (begin_chunk - data) * CHUNK_SIZE<T>;
    }

    size_type __back_capacity() {
        if (*end_iterator.outer_pointer == nullptr) {
            return 0;
        }
        return CHUNK_SIZE<T> - std::distance(*end_iterator.outer_pointer, end_iterator.inner_pointer);
    }

    size_type __back_ghost_capacity() {
        return __back_capacity() + ((data + num_chunks) - end_chunk) * CHUNK_SIZE<T>;
    }

    size_type __get_num_active_chunks() {
        return num_elements == 0 ? 0 : 
            ((end_iterator - 1).outer_pointer + 1 - begin_iterator.outer_pointer);
    }

    pointer_allocator_type& __get_pointer_allocator() {
        return pointer_allocator;
    }
};
}