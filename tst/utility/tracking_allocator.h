#pragma once
#include <memory>
#include <atomic>
#include <functional>

namespace algo {
template <typename T>
std::atomic_ullong allocated;

template <typename T>
std::atomic_ullong deallocated;

template <typename T>
std::atomic_ullong constructed;

template <typename T>
std::atomic_ullong destroyed;

template <typename T>
std::atomic_int counter;

template <typename T>
std::atomic_bool should_throw;

template <typename T>
std::atomic_ullong count_down;

template <typename T>
class tracking_allocator {
private:
    std::allocator<T> allocator;
    
public:
    int id;
    
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;

    static void reset() {
        should_throw<T> = false;
        allocated<T>.store(0);
        deallocated<T>.store(0);
        constructed<T>.store(0);
        destroyed<T>.store(0);
        count_down<T> = 1LL << 50;
    }

    static void check() {
        EXPECT_EQ(allocated<T>.load(), deallocated<T>.load());
        EXPECT_EQ(constructed<T>.load(), destroyed<T>.load());
    }

    tracking_allocator() : id(counter<T>.fetch_add(1)) {
        count_down<T> = 1LL << 50;
    }

    template<typename U>
    tracking_allocator(const tracking_allocator<U>& other) noexcept : id(other.id) {}

    T* allocate(std::size_t n) {
        if (should_throw<T>) {
            throw std::bad_alloc();
        }
        allocated<T> += n;
        return allocator.allocate(n);
    }

    void deallocate(T* p, std::size_t n) {
        deallocated<T> += n;
        allocator.deallocate(p, n);
    }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        if (count_down<T> == 0) {
            throw std::bad_function_call();
        }
        count_down<T>--;
        constructed<U>++;
        ::new(p) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U* p){
        p->~U();
        destroyed<U>++;
    }

    static void set_construct_throw_count_down(std::size_t remain) {
        count_down<T> = remain;
    }

    static void set_allocate_throw(bool on) {
        should_throw<T> = on;
    }
    
    friend bool operator==(const tracking_allocator& alloc1, const tracking_allocator& alloc2) noexcept {
        return alloc1.id == alloc2.id;
    }
};
}