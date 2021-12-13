#include "gtest/gtest.h"
#include "utility/constructor_stub.h"
#include "utility/throw_constructor_stub.h"
#include "utility/stub_iterator.h"
#include "common.h"
#include <vector>
#include <iostream>

namespace {
    using namespace algo;
    static int SPECIAL_VALUE = 0xdeadbeef;
    static int SPECIAL_VALUE2 = 0xbeefbabe;
    static int SMALL_LIMIT = 10;

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
        for (int i = 0; i < length; i++) {
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
        uninitialized_move_safe(ptr, constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(ptr -> id, SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        delete ptr;
    }

    TEST_F(common_test, uninitialized_move_unsafe_single_test) {
        throw_constructor_stub* ptr = static_cast<throw_constructor_stub*>(::operator new(sizeof(throw_constructor_stub)));
        uninitialized_move_safe(ptr, throw_constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(ptr -> id, SPECIAL_VALUE);
        EXPECT_EQ(throw_constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(throw_constructor_stub::copy_constructor_invocation_count, 1);
        delete ptr;
    }

    TEST_F(common_test, uninitialized_move_safe_range_test) {
        constructor_stub* stubs = static_cast<constructor_stub*>(::operator new(SMALL_LIMIT * sizeof(constructor_stub)));
        uninitialized_move_safe(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, uninitialized_move_unsafe_range_test) {
        throw_constructor_stub* stubs = static_cast<throw_constructor_stub*>(::operator new(SMALL_LIMIT * sizeof(throw_constructor_stub)));
        uninitialized_move_safe(stub_iterator<throw_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<throw_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(throw_constructor_stub::move_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, move_safe_single_test) {
        constructor_stub* stub = new constructor_stub();
        move_safe(stub, constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(stub -> id, SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::move_assignment_operator_invocation_count, 1);
        EXPECT_EQ(constructor_stub::assignment_operator_invocation_count, 0);
        delete stub;
    }

    TEST_F(common_test, move_unsafe_single_test) {
        throw_constructor_stub* stub = new throw_constructor_stub();
        move_safe(stub, throw_constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(stub -> id, SPECIAL_VALUE);
        EXPECT_EQ(throw_constructor_stub::move_assignment_operator_invocation_count, 0);
        EXPECT_EQ(throw_constructor_stub::assignment_operator_invocation_count, 1);
        delete stub;
    }

    TEST_F(common_test, move_safe_range_test) {
        constructor_stub stubs[SMALL_LIMIT];
        move_safe(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(constructor_stub::assignment_operator_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
    }

    TEST_F(common_test, move_unsafe_range_test) {
        throw_constructor_stub stubs[SMALL_LIMIT];
        move_safe(stub_iterator<throw_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<throw_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(throw_constructor_stub::move_assignment_operator_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
    }

    TEST_F(common_test, move_construct_safe_range_test) {
        constructor_stub* stubs = move_construct_safe(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, move_construct_unsafe_range_test) {
        throw_constructor_stub* stubs = move_construct_safe(stub_iterator<throw_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<throw_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), SMALL_LIMIT);
        EXPECT_EQ(throw_constructor_stub::move_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }
}