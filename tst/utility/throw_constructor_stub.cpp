#include <iostream>
#include <vector>
#include <utility>
#include "throw_constructor_stub.h"

namespace algo {

int throw_constructor_stub::default_constructor_invocation_count = 0;
int throw_constructor_stub::copy_constructor_invocation_count = 0;
int throw_constructor_stub::move_constructor_invocation_count = 0;
int throw_constructor_stub::assignment_operator_invocation_count = 0;
int throw_constructor_stub::move_assignment_operator_invocation_count = 0;
int throw_constructor_stub::destructor_invocation_count = 0;
int throw_constructor_stub::constructor_invocation_count = 0;

int throw_constructor_stub::counter = 0;

throw_constructor_stub::throw_constructor_stub() {
    id = counter++;
    default_constructor_invocation_count++;
    constructor_invocation_count++;
}

throw_constructor_stub::throw_constructor_stub(int id) {
    this -> id = id;
    constructor_invocation_count++;
}

throw_constructor_stub::throw_constructor_stub(const throw_constructor_stub& other) {
    id = other.id;
    copy_constructor_invocation_count++;
    constructor_invocation_count++;
}

throw_constructor_stub::throw_constructor_stub(throw_constructor_stub&& other) {
    id = other.id;
    move_constructor_invocation_count++;
    constructor_invocation_count++;
}

throw_constructor_stub& throw_constructor_stub::operator=(const throw_constructor_stub& other) {
    id = other.id;
    assignment_operator_invocation_count++;
    return *this;
}

throw_constructor_stub& throw_constructor_stub::operator=(throw_constructor_stub&& other) {
    id = other.id;
    move_assignment_operator_invocation_count++;
    return *this;
}

void throw_constructor_stub::reset_constructor_destructor_counter() noexcept {
    default_constructor_invocation_count = 0;
    copy_constructor_invocation_count = 0;
    move_constructor_invocation_count = 0;
    assignment_operator_invocation_count = 0;
    move_assignment_operator_invocation_count = 0;
    destructor_invocation_count = 0;
    constructor_invocation_count = 0;
}

throw_constructor_stub::~throw_constructor_stub() noexcept {
    destructor_invocation_count++;
}

bool operator==(const throw_constructor_stub& a, const throw_constructor_stub& b) {
    return a.id == b.id;
}
}
