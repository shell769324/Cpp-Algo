#include "gtest/gtest.h"
#include "src/stack.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/forward_stub.h"
#include <stack>
#include <deque>


namespace {
    using namespace algo;

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int LIMIT = 10000;
    static const int MEDIUM_LIMIT = 500;
    static const int SMALL_LIMIT = 10;

    template<typename T>
    class stack_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    template<typename T, typename Container>
    std::stack<T> convert_to_std_stack(stack<T, Container>& st) {
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

    template<typename T, typename U>
    void test_st_std_st_equality(stack<T, U>& st, std::stack<T>& std_st) {
        EXPECT_EQ(st.size(), std_st.size());
        stack<T, U> st_copy(st);
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
        for (int i = 0; i < limit; ++i) {
            st.emplace();
        }
        return st;
    }

    // Rely on function template deduction to figure out the Container and Args type
    template <typename T, template <typename ...> typename Container, typename... Args>
    auto rebind(Container<Args...>) {
        return Container<T>();
    };

    // Get the type of a container with its value type swapped to T
    template<typename T, typename Container>
    using morph = decltype(rebind<T>(std::declval<Container>()));

    TYPED_TEST_SUITE_P(stack_test);

    TYPED_TEST_P(stack_test, default_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        EXPECT_TRUE(st.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    
    TYPED_TEST_P(stack_test, copy_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        stack<constructor_stub, Container> st_copy(st);
        test_st_std_st_equality(st, std_st);
        test_st_std_st_equality(st_copy, std_st);
    }

    TYPED_TEST_P(stack_test, move_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        stack<constructor_stub, Container> st_copy(std::move(st));
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_st_std_st_equality(st_copy, std_st);
        EXPECT_TRUE(st.empty());
    }

    TYPED_TEST_P(stack_test, fill_default_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st(SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TYPED_TEST_P(stack_test, fill_copy_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        constructor_stub stub;
        stack<constructor_stub, Container> st(SMALL_LIMIT, stub);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            EXPECT_EQ(stub, st.top());
            st.pop();
        }
        EXPECT_TRUE(st.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, SMALL_LIMIT);
    }

    TYPED_TEST_P(stack_test, container_copy_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        Container source;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            source.emplace_back(i);
        }
        int copy_constructor_mark = constructor_stub::copy_constructor_invocation_count;
        int move_constructor_mark = constructor_stub::move_constructor_invocation_count;
        stack<constructor_stub, Container> st(source);
        EXPECT_EQ(copy_constructor_mark + SMALL_LIMIT, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(move_constructor_mark, constructor_stub::move_constructor_invocation_count);
        std::stack<constructor_stub> std_st(source.begin(), source.end());
        test_st_std_st_equality(st, std_st);
    }

    TYPED_TEST_P(stack_test, container_move_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        Container source;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            source.emplace_back(i);
        }
        std::stack<constructor_stub> std_st(source.begin(), source.end());
        int constructor_invocation_count = constructor_stub::constructor_invocation_count;
        stack<constructor_stub, Container> st(std::move(source));
        EXPECT_EQ(constructor_invocation_count, constructor_stub::constructor_invocation_count);
        test_st_std_st_equality(st, std_st);
    }

    TYPED_TEST_P(stack_test, range_constructor_test) {
        using Container = morph<constructor_stub, TypeParam>;
        std::deque<constructor_stub> std_deq(MEDIUM_LIMIT);
        stack<constructor_stub, Container> st(std_deq.begin(), std_deq.end());
        std::stack<constructor_stub> std_st(std_deq);
        test_st_std_st_equality(st, std_st);
        std_deq.clear();
        stack<constructor_stub, Container>().swap(st);
        std::stack<constructor_stub>().swap(std_st);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
    }

    TYPED_TEST_P(stack_test, initializer_constructor_test) {
        using Container = morph<int, TypeParam>;
        std::stack<int> std_st;
        for (int i = 1; i <= 5; ++i) {
            std_st.push(i);
        }
        stack<int, Container> st{1, 2, 3, 4, 5};
        test_st_std_st_equality(st, std_st);
    }

    TYPED_TEST_P(stack_test, assignment_operator_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        stack<constructor_stub, Container> st_copy;
        st_copy = st;
        test_st_std_st_equality(st, std_st);
        test_st_std_st_equality(st_copy, std_st);
    }

    TYPED_TEST_P(stack_test, move_assignment_operator_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        stack<constructor_stub, Container> st_copy;
        st_copy = std::move(st);
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_st_std_st_equality(st_copy, std_st);
    }

    TYPED_TEST_P(stack_test, push_primitive_test) {
        using Container = morph<int, TypeParam>;
        stack<int, Container> st;
        st.push(1);
        EXPECT_EQ(st.top(), 1);
    }


    TYPED_TEST_P(stack_test, push_lvalue_basic_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        constructor_stub constructor_stub;
        st.push(constructor_stub);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(st.size(), 1);
    }

    TYPED_TEST_P(stack_test, push_rvalue_basic_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        st.push(constructor_stub());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(st.size(), 1);
    }

    TYPED_TEST_P(stack_test, push_primitive_stress_test) {
        using Container = morph<int, TypeParam>;
        stack<int, Container> st;
        for (int i = 0; i < LIMIT; ++i) {
            st.push(i);
        }
        for (int i = LIMIT - 1; i >= 0; --i) {
            EXPECT_EQ(st.top(), i);
            st.pop();
        }
        EXPECT_TRUE(st.empty());
    }

    TYPED_TEST_P(stack_test, push_stress_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        for (int i = 0; i < LIMIT; ++i) {
            st.push(constructor_stub(i));
        }
        for (int i = LIMIT - 1; i >= 0; --i) {
            EXPECT_EQ(st.top().id, i);
            st.pop();
        }
        EXPECT_TRUE(st.empty());
    }

    TYPED_TEST_P(stack_test, pop_basic_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        st.push(constructor_stub());
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 1);
        st.pop();
        // Once when the temporary constructor_stub goes out of scope
        // Once in pop()
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 2);
        EXPECT_TRUE(st.empty());
    }

    TYPED_TEST_P(stack_test, pop_stress_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        for (int i = 0; i < LIMIT; ++i) {
            st.push(constructor_stub());
        }
        for (int i = 0; i < LIMIT; ++i) {
            st.pop();
        }
        EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
    }

    TYPED_TEST_P(stack_test, push_pop_stress_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        std::stack<constructor_stub> st_copy;
        int rep = 20;
        int limit = 1000;
        for (int i = 0; i < rep; ++i) {
            for (int j = 0; j < limit; ++j) {
                constructor_stub constructor_stub;
                st.push(constructor_stub);
                st_copy.push(constructor_stub);
            }
            std::size_t half_size = st.size() / 2;
            for (std::size_t j = 0; j < half_size; ++j) {
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

    TYPED_TEST_P(stack_test, empty_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        EXPECT_TRUE(st.empty());
        st.push(constructor_stub());
        EXPECT_FALSE(st.empty());
        st.pop();
        EXPECT_TRUE(st.empty());
    }

    TYPED_TEST_P(stack_test, size_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        for (int i = 0; i < LIMIT; ++i) {
            st.push(constructor_stub());
            EXPECT_EQ(st.size(), i + 1);
        }
        for (int i = 0; i < LIMIT; ++i) {
            st.pop();
            EXPECT_EQ(st.size(), LIMIT - i - 1);
        }
    }


    TYPED_TEST_P(stack_test, top_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            st.push(constructor_stub(i));
        }
        EXPECT_EQ(st.top().id, SMALL_LIMIT - 1);
    }

    TYPED_TEST_P(stack_test, top_alias_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            st.push(constructor_stub(i));
        }
        st.top().id = SPECIAL_VALUE;
        EXPECT_EQ(st.top().id, SPECIAL_VALUE);
    }


    TYPED_TEST_P(stack_test, emplace_forward_test) {
        using Container = morph<forward_stub, TypeParam>;
        stack<forward_stub, Container> st;
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

    TYPED_TEST_P(stack_test, emplace_return_value_test) {
        using Container = morph<forward_stub, TypeParam>;
        stack<forward_stub, Container> st;
        forward_stub& stub = st.emplace(SPECIAL_VALUE);
        EXPECT_EQ(st.top().n, SPECIAL_VALUE);
        ++stub.n;
        EXPECT_EQ(st.top().n, SPECIAL_VALUE + 1);
    }

    TYPED_TEST_P(stack_test, swap_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> st(SMALL_LIMIT);
        std::stack<constructor_stub> std_st = convert_to_std_stack(st);
        stack<constructor_stub, Container> st2(MEDIUM_LIMIT);
        std::stack<constructor_stub> std_st2 = convert_to_std_stack(st2);
        st.swap(st2);
        test_st_std_st_equality(st, std_st2);
        test_st_std_st_equality(st2, std_st);
    }

    template<typename Container>
    stack<constructor_stub, Container> to_stub_stack(deque<int> vec) {
        stack<constructor_stub, Container> stubs;
        for (int num : vec) {
            stubs.emplace(num);
        }
        return stubs;
    }

    TYPED_TEST_P(stack_test, equality_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> stack1 = to_stub_stack<Container>({0, 1, 2});
        stack<constructor_stub, Container> stack2 = to_stub_stack<Container>({0, 1, 2});
        EXPECT_EQ(stack1, stack2);
    }

    TYPED_TEST_P(stack_test, inequality_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> stack1 = to_stub_stack<Container>({0, 1, 2});
        stack<constructor_stub, Container> stack2 = to_stub_stack<Container>({0, 1, 3});
        EXPECT_NE(stack1, stack2);
    }

    TYPED_TEST_P(stack_test, three_way_comparison_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> stack1 = to_stub_stack<Container>({0, 1, 2});
        stack<constructor_stub, Container> stack2 = to_stub_stack<Container>({0, 1, 3});
        EXPECT_LT(stack1, stack2);
        EXPECT_LE(stack1, stack2);
        EXPECT_GT(stack2, stack1);
        EXPECT_GE(stack2, stack1);
    }

    TYPED_TEST_P(stack_test, three_way_comparison_length_test) {
        using Container = morph<constructor_stub, TypeParam>;
        stack<constructor_stub, Container> stack1 = to_stub_stack<Container>({0, 1});
        stack<constructor_stub, Container> stack2 = to_stub_stack<Container>({0, 1, 3});
        EXPECT_LT(stack1, stack2);
        EXPECT_LE(stack1, stack2);
        EXPECT_GT(stack2, stack1);
        EXPECT_GE(stack2, stack1);
    }

    REGISTER_TYPED_TEST_SUITE_P(stack_test,
        default_constructor_test,
        copy_constructor_test,
        move_constructor_test,
        fill_default_constructor_test,
        fill_copy_constructor_test,
        container_copy_constructor_test,
        container_move_constructor_test,
        range_constructor_test,
        initializer_constructor_test,
        assignment_operator_test,
        move_assignment_operator_test,
        push_primitive_test,
        push_lvalue_basic_test,
        push_rvalue_basic_test,
        push_primitive_stress_test,
        push_stress_test,
        pop_basic_test,
        pop_stress_test,
        push_pop_stress_test,
        empty_test,
        size_test,
        top_test,
        top_alias_test,
        emplace_forward_test,
        emplace_return_value_test,
        swap_test,
        equality_test,
        inequality_test,
        three_way_comparison_test,
        three_way_comparison_length_test
    );
}
