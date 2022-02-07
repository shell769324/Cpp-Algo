#pragma once

#include <memory>
#include <iterator>
#include "constructor_stub.h"

namespace algo {

template<typename T>
concept id_stub =requires(T& stub) {
    { stub.id } -> std::convertible_to<int>;
};

template<id_stub T> 
class stub_iterator {
private:
    std::shared_ptr<T> stub;

public:
    using iterator_category = std::input_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = std::ptrdiff_t;

    stub_iterator() = default;

    stub_iterator(const T& stub) {
        this -> stub = std::shared_ptr<T>(new T(stub));
    }

    stub_iterator(std::size_t stub_id) {
        this -> stub = std::shared_ptr<T>(new T(stub_id));
    }
    
    reference operator*() const noexcept {
        return *stub;
    }

    stub_iterator& operator++() noexcept {
        stub -> id++;
        return *this;
    }

    stub_iterator operator++(int) noexcept {
        stub_iterator res = *this;
        stub -> id++;
        return res;
    }

    bool operator< (const stub_iterator& other) const noexcept {
        return stub -> id < other.stub -> id;
    }

    bool operator<= (const stub_iterator& other) const noexcept {
        return stub -> id <= other.stub -> id;
    }

    bool operator> (const stub_iterator& other) const noexcept {
        return stub -> id > other.stub -> id;
    }

    bool operator>= (const stub_iterator& other) const noexcept {
        return stub -> id >= other.stub -> id;
    }

    bool operator== (const stub_iterator& other) const noexcept {
        return stub -> id == other.stub -> id;
    }
};
}
