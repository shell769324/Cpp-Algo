#include <iostream>
#include <vector>
#include <utility>
#include "move_only_constructor_stub.h"

namespace algo {

int move_only_constructor_stub::default_constructor_invocation_count = 0;
int move_only_constructor_stub::move_constructor_invocation_count = 0;
int move_only_constructor_stub::move_assignment_operator_invocation_count = 0;
int move_only_constructor_stub::destructor_invocation_count = 0;
int move_only_constructor_stub::constructor_invocation_count = 0;

int move_only_constructor_stub::counter = 0;

move_only_constructor_stub::move_only_constructor_stub() {
    id = counter++;
    default_constructor_invocation_count++;
    constructor_invocation_count++;
}

move_only_constructor_stub::move_only_constructor_stub(int id) {
    this -> id = id;
    constructor_invocation_count++;
}

move_only_constructor_stub::move_only_constructor_stub(move_only_constructor_stub&& other) {
    id = other.id;
    move_constructor_invocation_count++;
    constructor_invocation_count++;
}

move_only_constructor_stub& move_only_constructor_stub::operator=(move_only_constructor_stub&& other) {
    id = other.id;
    move_assignment_operator_invocation_count++;
    return *this;
}

void move_only_constructor_stub::reset_constructor_destructor_counter() noexcept {
    default_constructor_invocation_count = 0;
    move_constructor_invocation_count = 0;
    move_assignment_operator_invocation_count = 0;
    destructor_invocation_count = 0;
    constructor_invocation_count = 0;
}

move_only_constructor_stub::~move_only_constructor_stub() noexcept {
    destructor_invocation_count++;
}

bool operator==(const move_only_constructor_stub& a, const move_only_constructor_stub& b) {
    return a.id == b.id;
}

std::strong_ordering operator<=>(const move_only_constructor_stub& a, const move_only_constructor_stub& b) {
    return a.id <=> b.id;
}
}
