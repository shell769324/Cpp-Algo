#include "gtest/gtest.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/throw_constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "common.h"
#include <vector>
#include <iostream>

namespace {
    using namespace algo;
    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SMALL_LIMIT = 10;

    class common_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
            throw_constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            EXPECT_EQ(throw_constructor_stub::constructor_invocation_count, throw_constructor_stub::destructor_invocation_count);
        }
    };

    template<typename T>
    void verify_stub_ids(T* stubs, std::size_t length, int start) {
        for (std::size_t i = 0; i < length; i++) {
            EXPECT_EQ(stubs[i].id, start + i);
        }
    }

    template<typename T>
    void destroy_stubs(T* stubs, std::size_t length) {
        std::destroy(stubs, stubs + length);
        ::operator delete(stubs);
    }

    TEST_F(common_test, uninitialized_move_safe_single_test) {
        constructor_stub* ptr = static_cast<constructor_stub*>(::operator new(sizeof(constructor_stub)));
        safe_uninitialized_move(ptr, constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(ptr -> id, SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        delete ptr;
    }

    TEST_F(common_test, uninitialized_move_unsafe_single_test) {
        throw_constructor_stub* ptr = static_cast<throw_constructor_stub*>(::operator new(sizeof(throw_constructor_stub)));
        safe_uninitialized_move(ptr, throw_constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(ptr -> id, SPECIAL_VALUE);
        EXPECT_EQ(throw_constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(throw_constructor_stub::copy_constructor_invocation_count, 1);
        delete ptr;
    }

    TEST_F(common_test, uninitialized_move_safe_range_test) {
        constructor_stub* stubs = static_cast<constructor_stub*>(::operator new(SMALL_LIMIT * sizeof(constructor_stub)));
        safe_uninitialized_move(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, uninitialized_move_unsafe_range_test) {
        throw_constructor_stub* stubs = static_cast<throw_constructor_stub*>(::operator new(SMALL_LIMIT * sizeof(throw_constructor_stub)));
        safe_uninitialized_move(stub_iterator<throw_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<throw_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(throw_constructor_stub::move_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, move_safe_single_test) {
        constructor_stub* stub = new constructor_stub();
        safe_move(stub, constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(stub -> id, SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::move_assignment_operator_invocation_count, 1);
        EXPECT_EQ(constructor_stub::assignment_operator_invocation_count, 0);
        delete stub;
    }

    TEST_F(common_test, move_unsafe_single_test) {
        throw_constructor_stub* stub = new throw_constructor_stub();
        safe_move(stub, throw_constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(stub -> id, SPECIAL_VALUE);
        EXPECT_EQ(throw_constructor_stub::move_assignment_operator_invocation_count, 0);
        EXPECT_EQ(throw_constructor_stub::assignment_operator_invocation_count, 1);
        delete stub;
    }

    TEST_F(common_test, move_safe_range_test) {
        constructor_stub stubs[SMALL_LIMIT];
        safe_move(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(constructor_stub::assignment_operator_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
    }

    TEST_F(common_test, move_unsafe_range_test) {
        throw_constructor_stub stubs[SMALL_LIMIT];
        safe_move(stub_iterator<throw_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<throw_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(throw_constructor_stub::move_assignment_operator_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
    }

    TEST_F(common_test, move_construct_safe_range_test) {
        constructor_stub* stubs = safe_move_construct(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, move_construct_unsafe_range_test) {
        throw_constructor_stub* stubs = safe_move_construct(stub_iterator<throw_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<throw_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), SMALL_LIMIT);
        EXPECT_EQ(throw_constructor_stub::move_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }
    
    TEST_F(common_test, pair_left_accessor_test) {
        std::pair<int, std::string> pair(SPECIAL_VALUE, std::string("hello"));
        pair_left_accessor<int, std::string> accessor;
        EXPECT_EQ(accessor(pair), SPECIAL_VALUE);
    }
}
