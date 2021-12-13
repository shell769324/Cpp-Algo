#include "gtest/gtest.h"
#include "stack.h"
#include "utility/constructor_stub.h"
#include "utility/forward_stub.h"
#include <iostream>
#include <stack>
#include <vector>
#include <deque>


namespace {
    using namespace algo;
    class stack_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }
    };

    static int SPECIAL_VALUE = 0xdeadbeef;
    static int SPECIAL_VALUE2 = 0xbeefbabe;
    static int BIG_PRIME = 7759;
    static int MEDIUM_PRIME = 443;
    static int SMALL_PRIME = 19;
    static int LIMIT = 10000;
    static int MEDIUM_LIMIT = 500;
    static int SMALL_LIMIT = 10;

    template<typename T>
    std::stack<T> convert_to_std_stack(stack<T>& st) {
        std::stack<T> work_station;
        std::stack<T> copy_st;
        while (!st.empty()) {
            work_station.push(st.top());
            st.pop();
        }
        while (!work_station.empty()) {
            st.push(work_station.top());
            copy_st.push(work_station.top());
            work_station.pop();
        }
        return copy_st;
    }

    template<typename T>
    void test_st_std_st_equality(stack<T>& st, std::stack<T>& std_st) {
        EXPECT_EQ(st.size(), std_st.size());
        stack<T> st_copy(st);
        std::stack<T> std_st_copy(std_st);
        while (!st_copy.empty()) {
            EXPECT_EQ(st_copy.top(), std_st_copy.top());
            st_copy.pop();
            std_st_copy.pop();
        }
    }

    template<typename T>
    std::stack<T> construct_std_stack(std::size_t limit) {
        std::stack<T> st;
        for (int i = 0; i < limit; i++) {
            st.emplace();
        }
        return st;
    }

    TEST_F(stack_test, default_constructor_test) {
        stack<constructor_stub> st;
        EXPECT_TRUE(st.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(stack_test, copy_constructor_test) {
        stack<constructor_stub> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        stack<constructor_stub> st_copy(st);
        test_st_std_st_equality(st, std_st);
        test_st_std_st_equality(st_copy, std_st);
    }

    TEST_F(stack_test, move_constructor_test) {
        stack<constructor_stub> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        stack<constructor_stub> st_copy(std::move(st));
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_st_std_st_equality(st_copy, std_st);
        EXPECT_TRUE(st.empty());
    }

    TEST_F(stack_test, fill_default_constructor_test) {
        stack<constructor_stub> st(SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(stack_test, fill_copy_constructor_test) {
        constructor_stub stub;
        stack<constructor_stub> st(SMALL_LIMIT, stub);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            EXPECT_EQ(stub, st.top());
            st.pop();
        }
        EXPECT_TRUE(st.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, SMALL_LIMIT);
    }

    TEST_F(stack_test, range_constructor_test) {
        std::deque<constructor_stub> std_deq(MEDIUM_LIMIT);
        stack<constructor_stub> st(std_deq.begin(), std_deq.end());
        std::stack<constructor_stub> std_st(std_deq);
        test_st_std_st_equality(st, std_st);
        std_deq.clear();
        stack<constructor_stub>().swap(st);
        std::stack<constructor_stub>().swap(std_st);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
    }

    TEST_F(stack_test, initializer_constructor_test) {
        std::stack<int> std_st;
        for (int i = 1; i <= 5; i++) {
            std_st.push(i);
        }
        stack<int> st{1, 2, 3, 4, 5};
        test_st_std_st_equality(st, std_st);
    }

    TEST_F(stack_test, assignment_operator_test) {
        stack<constructor_stub> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        stack<constructor_stub> st_copy;
        st_copy = st;
        test_st_std_st_equality(st, std_st);
        test_st_std_st_equality(st_copy, std_st);
    }

    TEST_F(stack_test, move_assignment_operator_test) {
        stack<constructor_stub> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        stack<constructor_stub> st_copy;
        st_copy = std::move(st);
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_st_std_st_equality(st_copy, std_st);
    }

    TEST_F(stack_test, push_primitive_test) {
        stack<int> st;
        st.push(1);
        EXPECT_EQ(st.top(), 1);
    }


    TEST_F(stack_test, push_lvalue_basic_test) {
        stack<constructor_stub> st;
        constructor_stub constructor_stub;
        st.push(constructor_stub);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(st.size(), 1);
    }

    TEST_F(stack_test, push_rvalue_basic_test) {
        stack<constructor_stub> st;
        st.push(constructor_stub());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(st.size(), 1);
    }

    TEST_F(stack_test, push_primitive_stress_test) {
        stack<int> st;
        for (int i = 0; i < LIMIT; i++) {
            st.push(i);
        }
        for (int i = LIMIT - 1; i >= 0; i--) {
            EXPECT_EQ(st.top(), i);
            st.pop();
        }
        EXPECT_TRUE(st.empty());
    }

    TEST_F(stack_test, push_stress_test) {
        stack<constructor_stub> st;
        for (int i = 0; i < LIMIT; i++) {
            st.push(constructor_stub(i));
        }
        for (int i = LIMIT - 1; i >= 0; i--) {
            EXPECT_EQ(st.top().id, i);
            st.pop();
        }
        EXPECT_TRUE(st.empty());
    }

    TEST_F(stack_test, pop_basic_test) {
        stack<constructor_stub> st;
        st.push(constructor_stub());
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 1);
        st.pop();
        // Once when the temporary constructor_stub goes out of scope
        // Once in pop()
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 2);
        EXPECT_TRUE(st.empty());
    }

    TEST_F(stack_test, pop_stress_test) {
        stack<constructor_stub> st;
        for (int i = 0; i < LIMIT; i++) {
            st.push(constructor_stub());
        }
        for (int i = 0; i < LIMIT; i++) {
            st.pop();
        }
        EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
    }

    TEST_F(stack_test, push_pop_stress_test) {
        stack<constructor_stub> st;
        std::stack<constructor_stub> st_copy;
        int rep = 20;
        int limit = 1000;
        for (int i = 0; i < rep; i++) {
            for (int j = 0; j < limit; j++) {
                constructor_stub constructor_stub;
                st.push(constructor_stub);
                st_copy.push(constructor_stub);
            }
            std::size_t half_size = st.size() / 2;
            for (int j = 0; j < half_size; j++) {
                st.pop();
                st_copy.pop();
            }
            test_st_std_st_equality(st, st_copy);
        }
        while (!st.empty()) {
            st.pop();
            st_copy.pop();
        }
        EXPECT_EQ(st.size(), 0);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
    }

    TEST_F(stack_test, empty_test) {
        stack<constructor_stub> st;
        EXPECT_TRUE(st.empty());
        st.push(constructor_stub());
        EXPECT_FALSE(st.empty());
        st.pop();
        EXPECT_TRUE(st.empty());
    }

    TEST_F(stack_test, size_test) {
        stack<constructor_stub> st;
        for (int i = 0; i < LIMIT; i++) {
            st.push(constructor_stub());
            EXPECT_EQ(st.size(), i + 1);
        }
        for (int i = 0; i < LIMIT; i++) {
            st.pop();
            EXPECT_EQ(st.size(), LIMIT - i - 1);
        }
    }


    TEST_F(stack_test, back_test) {
        stack<constructor_stub> st;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            st.push(constructor_stub(i));
        }
        EXPECT_EQ(st.top().id, SMALL_LIMIT - 1);
    }

    TEST_F(stack_test, top_alias_test) {
        stack<constructor_stub> st;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            st.push(constructor_stub(i));
        }
        st.top().id = SPECIAL_VALUE;
        EXPECT_EQ(st.top().id, SPECIAL_VALUE);
    }


    TEST_F(stack_test, emplace_forward_test) {
        stack<forward_stub> st;
        st.emplace(SPECIAL_VALUE);
        EXPECT_EQ(st.top().construct_type,
            forward_stub::constructor_type::int_constructor);
        EXPECT_EQ(st.top().n, SPECIAL_VALUE);
        st.emplace(0.1f);
        EXPECT_EQ(st.top().construct_type,
            forward_stub::constructor_type::float_constructor);
        st.emplace(std::string());
        EXPECT_EQ(st.top().construct_type,
            forward_stub::constructor_type::string_constructor);
    }

    TEST_F(stack_test, emplace_return_value_test) {
        stack<forward_stub> st;
        forward_stub& stub = st.emplace(SPECIAL_VALUE);
        EXPECT_EQ(st.top().n, SPECIAL_VALUE);
        stub.n++;
        EXPECT_EQ(st.top().n, SPECIAL_VALUE + 1);
    }

    TEST_F(stack_test, swap_test) {
        stack<constructor_stub> st(SMALL_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        stack<constructor_stub> st2(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st2 = convert_to_std_stack(st2);
        st.swap(st2);
        test_st_std_st_equality(st, std_st2);
        test_st_std_st_equality(st2, std_st);
    }
}
