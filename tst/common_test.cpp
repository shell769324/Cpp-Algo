#include "gtest/gtest.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/copy_only_constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/utility/tracking_allocator.h"
#include "src/common.h"
#include <vector>

namespace {
    using namespace algo;
    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SMALL_LIMIT = 10;

    class common_test : public ::testing::Test {
    public:
        tracking_allocator<constructor_stub> allocator;
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
            copy_only_constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            EXPECT_EQ(copy_only_constructor_stub::constructor_invocation_count, copy_only_constructor_stub::destructor_invocation_count);
        }
    };

    template<typename T>
    void verify_stub_ids(T* stubs, std::size_t length, int start) {
        for (std::size_t i = 0; i < length; ++i) {
            EXPECT_EQ(stubs[i].id, start + i);
        }
    }

    template<typename T>
    void destroy_stubs(T* stubs, std::size_t length) {
        std::destroy(stubs, stubs + length);
        ::operator delete(stubs);
    }

    TEST_F(common_test, try_uninitialized_move_single_test) {
        constructor_stub* ptr = static_cast<constructor_stub*>(::operator new(sizeof(constructor_stub)));
        try_uninitialized_move(ptr, constructor_stub(SPECIAL_VALUE), allocator);
        EXPECT_EQ(ptr -> id, SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        delete ptr;
    }

    TEST_F(common_test, try_uninitialized_move_single_copy_test) {
        copy_only_constructor_stub* ptr = static_cast<copy_only_constructor_stub*>(::operator new(sizeof(copy_only_constructor_stub)));
        try_uninitialized_move(ptr, copy_only_constructor_stub(SPECIAL_VALUE), allocator);
        EXPECT_EQ(ptr -> id, SPECIAL_VALUE);
        EXPECT_EQ(copy_only_constructor_stub::copy_constructor_invocation_count, 1);
        delete ptr;
    }

    TEST_F(common_test, try_uninitialized_move_range_test) {
        constructor_stub* stubs = static_cast<constructor_stub*>(::operator new(SMALL_LIMIT * sizeof(constructor_stub)));
        try_uninitialized_move<true>(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs, allocator);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, try_uninitialized_move_range_copy_test) {
        copy_only_constructor_stub* stubs = static_cast<copy_only_constructor_stub*>(::operator new(SMALL_LIMIT * sizeof(copy_only_constructor_stub)));
        try_uninitialized_move<false>(stub_iterator<copy_only_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<copy_only_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs, allocator);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, try_move_single_test) {
        constructor_stub* stub = new constructor_stub();
        *stub = try_move(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(stub -> id, SPECIAL_VALUE);
        EXPECT_EQ(constructor_stub::move_assignment_operator_invocation_count, 1);
        EXPECT_EQ(constructor_stub::assignment_operator_invocation_count, 0);
        delete stub;
    }

    TEST_F(common_test, try_move_single_copy_test) {
        copy_only_constructor_stub* stub = new copy_only_constructor_stub();
        *stub = try_move(copy_only_constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(stub -> id, SPECIAL_VALUE);
        EXPECT_EQ(copy_only_constructor_stub::assignment_operator_invocation_count, 1);
        delete stub;
    }

    TEST_F(common_test, try_move_range_test) {
        constructor_stub stubs[SMALL_LIMIT];
        try_move<false>(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        EXPECT_EQ(constructor_stub::assignment_operator_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
    }

    TEST_F(common_test, try_move_range_copy_test) {
        copy_only_constructor_stub stubs[SMALL_LIMIT];
        try_move<true>(stub_iterator<copy_only_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<copy_only_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), stubs);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
    }

    TEST_F(common_test, try_move_construct_test) {
        constructor_stub* stubs = try_move_construct<false>(stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), SMALL_LIMIT, allocator);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }

    TEST_F(common_test, try_move_construct_copy_test) {
        tracking_allocator<copy_only_constructor_stub> copy_only_allocator;
        copy_only_constructor_stub* stubs = try_move_construct<true>(stub_iterator<copy_only_constructor_stub>(SPECIAL_VALUE),
            stub_iterator<copy_only_constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT), SMALL_LIMIT, copy_only_allocator);
        verify_stub_ids(stubs, SMALL_LIMIT, SPECIAL_VALUE);
        destroy_stubs(stubs, SMALL_LIMIT);
    }
    
    TEST_F(common_test, pair_left_accessor_test) {
        std::pair<int, std::string> pair(SPECIAL_VALUE, std::string("hello"));
        pair_left_accessor<int, std::string> accessor;
        EXPECT_EQ(accessor(pair), SPECIAL_VALUE);
    }
}
