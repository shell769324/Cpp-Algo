#include "gtest/gtest.h"
#include "construction_destruction_tracker.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/utility/move_only_constructor_stub.h"
#include "tst/utility/copy_only_constructor_stub.h"

namespace algo {
void construction_destruction_tracker::reset() {
    tracking_allocator<constructor_stub>::reset();
    tracking_allocator<copy_only_constructor_stub>::reset();
    tracking_allocator<move_only_constructor_stub>::reset();
    constructor_stub::reset_constructor_destructor_counter();
    copy_only_constructor_stub::reset_constructor_destructor_counter();
    move_only_constructor_stub::reset_constructor_destructor_counter();
}

void construction_destruction_tracker::check() {
    EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
    EXPECT_EQ(copy_only_constructor_stub::constructor_invocation_count, copy_only_constructor_stub::destructor_invocation_count);
    EXPECT_EQ(move_only_constructor_stub::constructor_invocation_count, move_only_constructor_stub::destructor_invocation_count);
    tracking_allocator<constructor_stub>::check();
    tracking_allocator<copy_only_constructor_stub>::check();
    tracking_allocator<move_only_constructor_stub>::check();
}

void construction_destruction_tracker::mark() {
    constructor_mark = constructor_stub::constructor_invocation_count;
    constructed_mark = constructed<constructor_stub>;
    destructor_mark = constructor_stub::destructor_invocation_count;
    destroyed_mark = destroyed<constructor_stub>;
    pair_constructed_mark = constructed<std::pair<const constructor_stub, constructor_stub>>;
    pair_destroyed_mark = destroyed<std::pair<const constructor_stub, constructor_stub>>;
}

void construction_destruction_tracker::check_marked() {
    int pair_constructed_count = constructed<std::pair<const constructor_stub, constructor_stub>> - pair_constructed_mark;
    int pair_destroyed_count = destroyed<std::pair<const constructor_stub, constructor_stub>> - pair_destroyed_mark;
    EXPECT_EQ(constructor_stub::constructor_invocation_count - constructor_mark, constructed<constructor_stub> - constructed_mark + pair_constructed_count * 2);
    EXPECT_EQ(constructor_stub::destructor_invocation_count - destructor_mark, destroyed<constructor_stub> - destroyed_mark + pair_destroyed_count * 2);
}
}