#include <iostream>
#include <vector>
#include <utility>
#include "constructor_stub.h"
#include <format>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

namespace algo {

int constructor_stub::default_constructor_invocation_count = 0;
int constructor_stub::id_constructor_invocation_count = 0;
int constructor_stub::copy_constructor_invocation_count = 0;
int constructor_stub::move_constructor_invocation_count = 0;
int constructor_stub::assignment_operator_invocation_count = 0;
int constructor_stub::move_assignment_operator_invocation_count = 0;
std::atomic_int constructor_stub::destructor_invocation_count = 0;
std::atomic_int constructor_stub::constructor_invocation_count = 0;

std::atomic_int constructor_stub::counter = 0;

unsigned long long MAGIC = 0xdeadffffeeeebeef;


constructor_stub::constructor_stub() {
    id = counter++;
    uid = std::rand();
    if (magic == MAGIC) {
        throw std::runtime_error(std::format("id {} stub was not correctly destructed", id));
    }
    magic = MAGIC;
    default_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub::constructor_stub(int id) {
    this -> id = id;
    uid = std::rand();
    if (magic == MAGIC) {
        throw std::runtime_error(std::format("id constructor: id {} stub was not correctly destructed", id));
    }
    magic = MAGIC;
    id_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub::constructor_stub(const constructor_stub& other) {
    id = other.id;
    uid = other.uid;
    if (magic == MAGIC) {
        throw std::runtime_error(std::format("copy constructor: id {} stub was not correctly destructed", id));
    }
    magic = MAGIC;
    copy_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub::constructor_stub(constructor_stub&& other) noexcept {
    if (magic == MAGIC) {
        std::cout << std::format("move constructor: id {} stub was not correctly destructed. Current counter {}\n", id,  counter.load());
    }
    id = other.id;
    uid = other.uid;
    magic = MAGIC;
    move_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub& constructor_stub::operator=(const constructor_stub& other) {
    if (magic != MAGIC) {
        throw std::runtime_error(std::format("copy assignment: id {} stub is not initialized yet", id));
    }
    id = other.id;
    uid = other.uid;
    magic = MAGIC;
    assignment_operator_invocation_count++;
    return *this;
}

constructor_stub& constructor_stub::operator=(constructor_stub&& other) noexcept {
    if (magic != MAGIC) {
        std::cout << std::format("move assignment: id {} stub is not initialized yet. Current counter {}\n", id, counter.load());
    }
    id = other.id;
    uid = other.uid;
    magic = MAGIC;
    move_assignment_operator_invocation_count++;
    return *this;
}

bool operator==(const constructor_stub& stub1, const constructor_stub& stub2) noexcept {
    return stub1.id == stub2.id;
}

std::strong_ordering operator<=>(const constructor_stub& stub1, const constructor_stub& stub2) noexcept {
    return stub1.id <=> stub2.id;
}

void constructor_stub::reset_constructor_destructor_counter() noexcept {
    default_constructor_invocation_count = 0;
    id_constructor_invocation_count = 0;
    copy_constructor_invocation_count = 0;
    move_constructor_invocation_count = 0;
    assignment_operator_invocation_count = 0;
    move_assignment_operator_invocation_count = 0;
    destructor_invocation_count = 0;
    constructor_invocation_count = 0;
}

constructor_stub::~constructor_stub() noexcept {
    if (magic != MAGIC) {
        std::cout << "Inside destructor id " << id << " stub is not initialized yet\n";
    }
    magic = 0;
    destructor_invocation_count++;
}

bool constructor_stub::is_valid() noexcept {
    return magic == MAGIC;
}

}
#pragma GCC diagnostic pop