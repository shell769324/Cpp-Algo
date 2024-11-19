#pragma once
#include <iostream>
#include <atomic>

namespace algo {

class constructor_stub {
public:
    static int default_constructor_invocation_count;
    static int id_constructor_invocation_count;
    static int copy_constructor_invocation_count;
    static int move_constructor_invocation_count;
    static int assignment_operator_invocation_count;
    static int move_assignment_operator_invocation_count;
    static std::atomic_int destructor_invocation_count;
    static std::atomic_int constructor_invocation_count;
    static std::atomic_int counter;


    int id;
    int uid;
    unsigned long long magic;

    constructor_stub();

    constructor_stub(int id);

    constructor_stub(const constructor_stub& other);

    constructor_stub(constructor_stub&& other) noexcept;

    constructor_stub& operator=(const constructor_stub& other);

    constructor_stub& operator=(constructor_stub&& other) noexcept;

    static void reset_constructor_destructor_counter() noexcept;
    
    bool is_valid() noexcept;

    friend bool operator==(const constructor_stub& stub1, const constructor_stub& stub2) noexcept;
    friend std::strong_ordering operator<=>(const constructor_stub& stub1, const constructor_stub& stub2) noexcept;

    ~constructor_stub() noexcept;
};

struct constructor_stub_key_getter {
    int operator()(const constructor_stub& stub) const {
        return stub.id;
    }
};

struct constructor_stub_comparator {
    constructor_stub_comparator() : reverse(false), id(constructor_stub::counter.fetch_add(1)) { }

    constructor_stub_comparator(bool reverse) : reverse(reverse), id(constructor_stub::counter.fetch_add(1)) { }

    bool operator()(const constructor_stub& stub1, const constructor_stub& stub2) const {
        if (reverse) {
            return stub1.id > stub2.id;
        }
        return stub1.id < stub2.id;
    }

    friend bool operator==(const constructor_stub_comparator& comp1, const constructor_stub_comparator& comp2) {
        return comp1.id == comp2.id && comp1.reverse == comp2.reverse;
    }

    bool reverse;
    int id;
};
}
