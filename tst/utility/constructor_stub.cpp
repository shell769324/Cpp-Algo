#include <iostream>
#include <vector>
#include <utility>
#include "constructor_stub.h"

namespace algo {

int constructor_stub::default_constructor_invocation_count = 0;
int constructor_stub::copy_constructor_invocation_count = 0;
int constructor_stub::move_constructor_invocation_count = 0;
int constructor_stub::assignment_operator_invocation_count = 0;
int constructor_stub::move_assignment_operator_invocation_count = 0;
int constructor_stub::destructor_invocation_count = 0;
int constructor_stub::constructor_invocation_count = 0;

int constructor_stub::counter = 0;

constructor_stub::constructor_stub() {
    id = counter++;
    default_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub::constructor_stub(int id) {
    this -> id = id;
    constructor_invocation_count++;
}

constructor_stub::constructor_stub(const constructor_stub& other) {
    id = other.id;
    copy_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub::constructor_stub(constructor_stub&& other) noexcept {
    id = other.id;
    move_constructor_invocation_count++;
    constructor_invocation_count++;
}

constructor_stub& constructor_stub::operator=(const constructor_stub& other) {
    id = other.id;
    assignment_operator_invocation_count++;
    return *this;
}

constructor_stub& constructor_stub::operator=(constructor_stub&& other) noexcept {
    id = other.id;
    move_assignment_operator_invocation_count++;
    return *this;
}

void constructor_stub::reset_constructor_destructor_counter() noexcept {
    default_constructor_invocation_count = 0;
    copy_constructor_invocation_count = 0;
    move_constructor_invocation_count = 0;
    assignment_operator_invocation_count = 0;
    move_assignment_operator_invocation_count = 0;
    destructor_invocation_count = 0;
    constructor_invocation_count = 0;
}

constructor_stub::~constructor_stub() noexcept {
    destructor_invocation_count++;
}

bool operator==(const constructor_stub& a, const constructor_stub& b) {
    return a.id == b.id;
}
}
