#include "gtest/gtest.h"
#include "tst/utility/construction_destruction_tracker.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/copy_only_constructor_stub.h"
#include "tst/utility/move_only_constructor_stub.h"
#include "src/allocator_aware_algorithms.h"

namespace {
    using namespace algo;
    class allcator_aware_algorithms_test : public ::testing::Test {
        construction_destruction_tracker tracker;
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            tracker.reset();
        }

        virtual void TearDown() {
            tracker.check();
        }
    };

    static const int SMALL_LIMIT = 10;
    static const int SPECIAL_VALUE = 0xdeadbeef;

    template<typename T>
    void destroy_test_helper() {
        tracking_allocator<T> allocator;
        T* pointer = allocator.allocate(SMALL_LIMIT);
        for (T* curr = pointer, *end = pointer + SMALL_LIMIT; curr != end; curr++) {
            allocator.construct(curr, curr - pointer);
        }
        destroy(pointer, pointer + SMALL_LIMIT, allocator);
        EXPECT_EQ(T::destructor_invocation_count, SMALL_LIMIT);
        allocator.deallocate(pointer, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, destroy_regular_test) {
        destroy_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, destroy_copy_only_test) {
        destroy_test_helper<copy_only_constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, destroy_move_only_test) {
        destroy_test_helper<move_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_default_construct_test_helper() {
        tracking_allocator<T> allocator;
        T* pointer = allocator.allocate(SMALL_LIMIT);
        uninitialized_default_construct(pointer, pointer + SMALL_LIMIT, allocator);
        EXPECT_EQ(T::default_constructor_invocation_count, SMALL_LIMIT);
        destroy(pointer, pointer + SMALL_LIMIT, allocator);
        allocator.deallocate(pointer, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_default_construct_regular_test) {
        uninitialized_default_construct_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_default_construct_copy_only_test) {
        uninitialized_default_construct_test_helper<copy_only_constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_default_construct_move_only_test) {
        uninitialized_default_construct_test_helper<move_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_default_construct_exception_test_helper() {
        tracking_allocator<T> allocator;
        T* pointer = allocator.allocate(SMALL_LIMIT);
        int half = SMALL_LIMIT / 2;
        allocator.set_construct_throw_count_down(half);
        try {
            uninitialized_default_construct(pointer, pointer + SMALL_LIMIT, allocator);
        } catch (const std::bad_function_call& e) {
            allocator.set_construct_throw_count_down(1L << 50);
        }
        EXPECT_EQ(T::default_constructor_invocation_count, half);
        EXPECT_EQ(T::destructor_invocation_count, half);
        allocator.deallocate(pointer, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_default_construct_regular_exception_test) {
        uninitialized_default_construct_exception_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_default_construct_copy_only_exception_test) {
        uninitialized_default_construct_exception_test_helper<copy_only_constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_default_construct_move_only_exception_test) {
        uninitialized_default_construct_exception_test_helper<move_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_fill_test_helper() {
        tracking_allocator<T> allocator;
        T* pointer = allocator.allocate(SMALL_LIMIT);
        T stub(SPECIAL_VALUE);
        uninitialized_fill(pointer, pointer + SMALL_LIMIT, stub, allocator);
        EXPECT_EQ(T::copy_constructor_invocation_count, SMALL_LIMIT);
        destroy(pointer, pointer + SMALL_LIMIT, allocator);
        allocator.deallocate(pointer, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_fill_regular_test) {
        uninitialized_fill_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_fill_copy_only_test) {
        uninitialized_fill_test_helper<copy_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_fill_exception_test_helper() {
        tracking_allocator<T> allocator;
        T* pointer = allocator.allocate(SMALL_LIMIT);
        T stub(SPECIAL_VALUE);
        int half = SMALL_LIMIT / 2;
        allocator.set_construct_throw_count_down(half);
        try {
            uninitialized_fill(pointer, pointer + SMALL_LIMIT, stub, allocator);
        } catch (const std::bad_function_call& e) {
            allocator.set_construct_throw_count_down(1L << 50);
        }
        EXPECT_EQ(T::copy_constructor_invocation_count, half);
        EXPECT_EQ(T::destructor_invocation_count, half);
        allocator.deallocate(pointer, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_fill_regular_exception_test) {
        uninitialized_fill_exception_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_fill_copy_only_exception_test) {
        uninitialized_fill_exception_test_helper<copy_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_copy_test_helper() {
        tracking_allocator<T> allocator;
        T* dest = allocator.allocate(SMALL_LIMIT);
        T* src = allocator.allocate(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            allocator.construct(src + i, i);
        }
        T* out = uninitialized_copy(src, src + SMALL_LIMIT, dest, allocator);
        EXPECT_EQ(dest + SMALL_LIMIT, out);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            EXPECT_EQ(src[i], dest[i]);
        }
        EXPECT_EQ(T::copy_constructor_invocation_count, SMALL_LIMIT);
        destroy(src, src + SMALL_LIMIT, allocator);
        destroy(dest, dest + SMALL_LIMIT, allocator);
        allocator.deallocate(src, SMALL_LIMIT);
        allocator.deallocate(dest, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_copy_regular_test) {
        uninitialized_copy_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_copy_copy_only_test) {
        uninitialized_copy_test_helper<copy_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_copy_exception_test_helper() {
        tracking_allocator<T> allocator;
        T* dest = allocator.allocate(SMALL_LIMIT);
        T* src = allocator.allocate(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            allocator.construct(src + i, i);
        }
        int half = SMALL_LIMIT / 2;
        allocator.set_construct_throw_count_down(half);
        try {
            uninitialized_copy(src, src + SMALL_LIMIT, dest, allocator);
        } catch (const std::bad_function_call& e) {
            allocator.set_construct_throw_count_down(1L << 50);
        }
        EXPECT_EQ(T::copy_constructor_invocation_count, half);
        EXPECT_EQ(T::destructor_invocation_count, half);
        destroy(src, src + SMALL_LIMIT, allocator);
        allocator.deallocate(src, SMALL_LIMIT);
        allocator.deallocate(dest, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_copy_regular_exception_test) {
        uninitialized_copy_exception_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_copy_copy_only_exception_test) {
        uninitialized_copy_exception_test_helper<copy_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_move_test_helper() {
        tracking_allocator<T> allocator;
        T* dest = allocator.allocate(SMALL_LIMIT);
        T* src = allocator.allocate(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            allocator.construct(src + i, i);
        }
        T* out = uninitialized_move(src, src + SMALL_LIMIT, dest, allocator);
        EXPECT_EQ(dest + SMALL_LIMIT, out);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            EXPECT_EQ(i, dest[i].id);
        }
        EXPECT_EQ(T::move_constructor_invocation_count, SMALL_LIMIT);
        destroy(src, src + SMALL_LIMIT, allocator);
        destroy(dest, dest + SMALL_LIMIT, allocator);
        allocator.deallocate(src, SMALL_LIMIT);
        allocator.deallocate(dest, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_move_regular_test) {
        uninitialized_move_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_move_move_only_test) {
        uninitialized_move_test_helper<move_only_constructor_stub>();
    }

    template<typename T>
    void uninitialized_move_exception_test_helper() {
        tracking_allocator<T> allocator;
        T* dest = allocator.allocate(SMALL_LIMIT);
        T* src = allocator.allocate(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            allocator.construct(src + i, i);
        }
        int half = SMALL_LIMIT / 2;
        allocator.set_construct_throw_count_down(half);
        try {
            uninitialized_move(src, src + SMALL_LIMIT, dest, allocator);
        } catch (const std::bad_function_call& e) {
            allocator.set_construct_throw_count_down(1L << 50);
        }
        EXPECT_EQ(T::move_constructor_invocation_count, half);
        EXPECT_EQ(T::destructor_invocation_count, half);
        destroy(src, src + SMALL_LIMIT, allocator);
        allocator.deallocate(src, SMALL_LIMIT);
        allocator.deallocate(dest, SMALL_LIMIT);
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_move_regular_exception_test) {
        uninitialized_move_exception_test_helper<constructor_stub>();
    }

    TEST_F(allcator_aware_algorithms_test, uninitialized_move_move_only_exception_test) {
        uninitialized_move_exception_test_helper<move_only_constructor_stub>();
    }
}