#include <iostream>
#include <vector>
#include <utility>
#include "copy_only_constructor_stub.h"

namespace algo {

int copy_only_constructor_stub::default_constructor_invocation_count = 0;
int copy_only_constructor_stub::copy_constructor_invocation_count = 0;
int copy_only_constructor_stub::assignment_operator_invocation_count = 0;
int copy_only_constructor_stub::destructor_invocation_count = 0;
int copy_only_constructor_stub::constructor_invocation_count = 0;

int copy_only_constructor_stub::counter = 0;

copy_only_constructor_stub::copy_only_constructor_stub() {
    id = counter++;
    default_constructor_invocation_count++;
    constructor_invocation_count++;
}

copy_only_constructor_stub::copy_only_constructor_stub(int id) {
    this -> id = id;
    constructor_invocation_count++;
}

copy_only_constructor_stub::copy_only_constructor_stub(const copy_only_constructor_stub& other) {
    id = other.id;
    copy_constructor_invocation_count++;
    constructor_invocation_count++;
}

copy_only_constructor_stub& copy_only_constructor_stub::operator=(const copy_only_constructor_stub& other) {
    id = other.id;
    assignment_operator_invocation_count++;
    return *this;
}

void copy_only_constructor_stub::reset_constructor_destructor_counter() noexcept {
    default_constructor_invocation_count = 0;
    copy_constructor_invocation_count = 0;
    assignment_operator_invocation_count = 0;
    destructor_invocation_count = 0;
    constructor_invocation_count = 0;
}

copy_only_constructor_stub::~copy_only_constructor_stub() noexcept {
    destructor_invocation_count++;
}

bool operator==(const copy_only_constructor_stub& a, const copy_only_constructor_stub& b) {
    return a.id == b.id;
}
}
