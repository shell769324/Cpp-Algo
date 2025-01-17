#include "gtest/gtest.h"
#include "src/vector/vector.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/forward_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/utility/common.h"
#include <vector>
#include "tst/utility/move_only_constructor_stub.h"
#include "tst/utility/copy_only_constructor_stub.h"
#include "tst/utility/construction_destruction_tracker.h"

namespace {
    using namespace algo;
    class vector_test : public ::testing::Test {
    protected:
        construction_destruction_tracker tracker;
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

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SPECIAL_VALUE2 = 0xbeefbabe;
    static const int BIG_PRIME = 7759;
    static const int MEDIUM_PRIME = 443;
    static const int SMALL_PRIME = 19;
    static const int LIMIT = 10000;
    static const int MEDIUM_LIMIT = 500;
    static const int SMALL_LIMIT = 10;
    static tracking_allocator<constructor_stub> allocator;
    using vector_type = vector<constructor_stub, tracking_allocator<constructor_stub> >;

    template<typename T>
    void test_vec_std_vec_equality(vector<T, tracking_allocator<T> >& vec, std::vector<T>& std_vec) {
        EXPECT_EQ(vec.size(), std_vec.size());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            EXPECT_EQ(vec[i], std_vec[i]);
        }
    }

    TEST_F(vector_test, trait_test) {
        static_assert(std::is_same_v<typename vector_type::value_type, constructor_stub>);
        static_assert(std::is_same_v<typename vector_type::allocator_type, tracking_allocator<constructor_stub> >);
        static_assert(std::is_same_v<typename vector_type::size_type, std::size_t>);
        static_assert(std::is_same_v<typename vector_type::difference_type, std::ptrdiff_t>);
        static_assert(std::is_same_v<typename vector_type::reference, constructor_stub&>);
        static_assert(std::is_same_v<typename vector_type::const_reference, const constructor_stub&>);
        static_assert(std::is_same_v<typename vector_type::pointer, constructor_stub*>);
        static_assert(std::is_same_v<typename vector_type::const_pointer, const constructor_stub*>);
        static_assert(std::is_same_v<typename vector_type::iterator, vector_iterator<constructor_stub> >);
        static_assert(std::is_same_v<typename vector_type::const_iterator, vector_iterator<const constructor_stub> >);
        static_assert(std::is_same_v<typename vector_type::reverse_iterator, vector_iterator<constructor_stub, true> >);
        static_assert(std::is_same_v<typename vector_type::const_reverse_iterator, vector_iterator<const constructor_stub, true> >);
    }

    void default_constructor_test_helper(vector_type& vec) {
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(vec.__get_default_capacity(), allocated<constructor_stub>);
    }

    TEST_F(vector_test, default_constructor_test) {
        vector_type vec;
        default_constructor_test_helper(vec);
    }

    TEST_F(vector_test, allocator_default_constructor_test) {
        vector_type vec(allocator);
        default_constructor_test_helper(vec);
        EXPECT_EQ(vec.get_allocator(), allocator);
    }

    void default_fill_constructor_test_helper(vector_type& vec) {
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(SMALL_LIMIT, allocated<constructor_stub>);
        EXPECT_EQ(SMALL_LIMIT, vec.capacity());
    }

    TEST_F(vector_test, default_fill_constructor_test) {
        this -> tracker.mark();
        vector_type vec(SMALL_LIMIT);
        this -> tracker.check_marked();
        default_fill_constructor_test_helper(vec);
    }

    TEST_F(vector_test, default_fill_constructor_optional_test) {
        vector_type vec(SMALL_LIMIT, allocator);
        default_fill_constructor_test_helper(vec);
        EXPECT_EQ(vec.get_allocator(), allocator);
    }

    void fill_constructor_test_helper(constructor_stub& stub, vector_type& vec) {
        for (auto& elem : vec) {
            EXPECT_EQ(stub, elem);
        }
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(SMALL_LIMIT, allocated<constructor_stub>);
        EXPECT_EQ(SMALL_LIMIT, vec.capacity());
    }

    TEST_F(vector_test, fill_constructor_test) {
        constructor_stub stub;
        this -> tracker.mark();
        vector_type vec(SMALL_LIMIT, stub, allocator);
        this -> tracker.check_marked();
        fill_constructor_test_helper(stub, vec);
        EXPECT_EQ(vec.get_allocator(), allocator);
    }

    TEST_F(vector_test, fill_constructor_optional_test) {
        constructor_stub stub;
        vector_type vec(SMALL_LIMIT, stub);
        fill_constructor_test_helper(stub, vec);
    }

    void range_constructor_test_helper(std::vector<constructor_stub>& std_vec, vector_type& vec) {
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, MEDIUM_LIMIT);
        // After insertion with resize, the vector should always half full
        EXPECT_EQ(allocated<constructor_stub>, MEDIUM_LIMIT * 2);
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, range_constructor_test) {
        std::vector<constructor_stub> std_vec(MEDIUM_LIMIT);
        this -> tracker.mark();
        vector_type vec(std_vec.begin(), std_vec.end(), allocator);
        this -> tracker.check_marked();
        range_constructor_test_helper(std_vec, vec);
        EXPECT_EQ(vec.get_allocator(), allocator);
    }

    TEST_F(vector_test, range_constructor_optional_test) {
        std::vector<constructor_stub> std_vec(MEDIUM_LIMIT);
        vector_type vec(std_vec.begin(), std_vec.end());
        range_constructor_test_helper(std_vec, vec);
    }

    TEST_F(vector_test, range_constructor_empty_range_test) {
        std::vector<constructor_stub> std_vec;
        vector_type vec(std_vec.begin(), std_vec.end(), allocator);
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(vec.capacity(), vec.__get_default_capacity());
        EXPECT_EQ(vec.get_allocator(), allocator);
    }

    TEST_F(vector_test, copy_constructor_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        std::size_t copy_count = constructor_stub::copy_constructor_invocation_count;
        std::size_t alloc_count = allocated<constructor_stub>;
        this -> tracker.mark();
        vector_type vec_copy(vec);
        this -> tracker.check_marked();
        EXPECT_EQ(copy_count + MEDIUM_LIMIT, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(alloc_count + MEDIUM_LIMIT, allocated<constructor_stub>);
        EXPECT_EQ(vec.capacity(), vec_copy.capacity());
        test_vec_std_vec_equality(vec, std_vec);
        test_vec_std_vec_equality(vec_copy, std_vec);
        for (int i = 0; i < MEDIUM_LIMIT * 2; ++i) {
            vec_copy.emplace_back(i);
            std_vec.emplace_back(i);
        }
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, allocator_copy_constructor_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        std::size_t copy_count = constructor_stub::copy_constructor_invocation_count;
        std::size_t alloc_count = allocated<constructor_stub>;
        vector_type vec_copy(vec, allocator);
        EXPECT_EQ(copy_count + MEDIUM_LIMIT, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(alloc_count + MEDIUM_LIMIT, allocated<constructor_stub>);
        EXPECT_EQ(vec.capacity(), vec_copy.capacity());
        test_vec_std_vec_equality(vec, std_vec);
        test_vec_std_vec_equality(vec_copy, std_vec);
        for (int i = 0; i < MEDIUM_LIMIT * 2; ++i) {
            vec_copy.emplace_back(i);
            std_vec.emplace_back(i);
        }
        test_vec_std_vec_equality(vec_copy, std_vec);
        EXPECT_EQ(vec_copy.get_allocator(), allocator);
    }

    TEST_F(vector_test, move_constructor_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        std::size_t constructor_count = constructor_stub::constructor_invocation_count;
        std::size_t alloc_count = allocated<constructor_stub>;
        vector_type vec_copy(std::move(vec));
        EXPECT_EQ(constructor_count, constructor_stub::constructor_invocation_count);
        EXPECT_EQ(alloc_count, allocated<constructor_stub>);
        test_vec_std_vec_equality(vec_copy, std_vec);
        for (int i = 0; i < MEDIUM_LIMIT * 2; ++i) {
            vec_copy.emplace_back(i);
            std_vec.emplace_back(i);
        }
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, allocator_move_constructor_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        std::size_t constructor_count = constructor_stub::constructor_invocation_count;
        std::size_t alloc_count = allocated<constructor_stub>;
        vector_type vec_copy(std::move(vec), allocator);
        EXPECT_EQ(constructor_count, constructor_stub::constructor_invocation_count);
        EXPECT_EQ(alloc_count, allocated<constructor_stub>);
        test_vec_std_vec_equality(vec_copy, std_vec);
        for (int i = 0; i < MEDIUM_LIMIT * 2; ++i) {
            vec_copy.emplace_back(i);
            std_vec.emplace_back(i);
        }
        test_vec_std_vec_equality(vec_copy, std_vec);
        EXPECT_EQ(vec_copy.get_allocator(), allocator);
    }

    TEST_F(vector_test, initializer_constructor_test) {
        std::vector<int> std_vec{1, 2, 3, 4, 5};
        vector<int, tracking_allocator<int> > vec{1, 2, 3, 4, 5};
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, assignment_operator_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        vector_type vec_copy;
        this -> tracker.mark();
        vec_copy = vec;
        this -> tracker.check_marked();
        test_vec_std_vec_equality(vec, std_vec);
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, assignment_operator_shortcut_test) {
        vector<int, tracking_allocator<int> > vec(SMALL_LIMIT);
        std::vector<int> std_vec(vec.begin(), vec.end());
        vector<int, tracking_allocator<int> > vec_copy(SMALL_LIMIT * 2);
        // No buffer allocated if the destination is big enough
        std::size_t alloc_count = allocated<int>;
        EXPECT_GT(alloc_count, 0);
        this -> tracker.mark();
        vec_copy = vec;
        this -> tracker.check_marked();
        EXPECT_EQ(alloc_count, allocated<int>);
        test_vec_std_vec_equality(vec, std_vec);
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, assignment_operator_operation_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        vector_type vec_copy;
        vec_copy = vec;
        for (int i = 0; i < MEDIUM_LIMIT * 2; ++i) {
            vec_copy.emplace_back(i);
            std_vec.emplace_back(i);
        }
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, move_assignment_operator_test) {
        vector_type vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        vector_type vec_copy;
        vec_copy = std::move(vec);
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, assign_fill_no_reallocation_test) {
        vector_type vec(MEDIUM_LIMIT);
        int allocation_count = allocated<constructor_stub>;
        vec.assign(MEDIUM_LIMIT / 2, SPECIAL_VALUE);
        EXPECT_EQ(allocation_count, allocated<constructor_stub>);
        EXPECT_EQ(MEDIUM_LIMIT / 2, vec.size());
        for (constructor_stub& stub : vec) {
            EXPECT_EQ(stub.id, SPECIAL_VALUE);
        }
    }

    TEST_F(vector_test, assign_fill_reallocation_test) {
        vector_type vec;
        vec.assign(MEDIUM_LIMIT, SPECIAL_VALUE);
        EXPECT_EQ(MEDIUM_LIMIT, vec.size());
        for (constructor_stub& stub : vec) {
            EXPECT_EQ(stub.id, SPECIAL_VALUE);
        }
    }

    TEST_F(vector_test, assign_fill_range_test) {
        vector_type vec;
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        vec.assign(stubs.begin(), stubs.end());
        test_vec_std_vec_equality(vec, stubs);
    }

    TEST_F(vector_test, push_back_primitive_test) {
        vector<int> vec;
        vec.push_back(1);
        EXPECT_EQ(vec[0], 1);
    }

    TEST_F(vector_test, push_back_lvalue_basic_test) {
        vector_type vec;
        constructor_stub constructor_stub;
        this -> tracker.mark();
        vec.push_back(constructor_stub);
        this -> tracker.check_marked();
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(vec.size(), 1);
    }

    template<typename T>
    void test_vec_std_vec_operability(vector<T, tracking_allocator<T> >& vec, std::vector<T>& standard) {
        test_vec_std_vec_equality(vec, standard);
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        for (auto& s : stubs) {
            vec.push_back(s);
            standard.push_back(s);
        }
        test_vec_std_vec_equality(vec, standard);
        vec.insert(vec.begin() + vec.size() / 2, stubs.begin(), stubs.end());
        standard.insert(standard.begin() + standard.size() / 2, stubs.begin(), stubs.end());
        test_vec_std_vec_equality(vec, standard);
    }

    std::vector<constructor_stub> set_up_for_exception(vector_type& vec) {
        std::vector<constructor_stub> std_vec;
        constructor_stub stub;
        while (vec.size() < vec.capacity()) {
            vec.push_back(stub);
            std_vec.push_back(vec.back());
        }
        return std_vec;
    }

    TEST_F(vector_test, push_back_exception_safety_test) {
        vector_type vec;
        std::vector<constructor_stub> std_vec = set_up_for_exception(vec);
        tracking_allocator<constructor_stub>::set_allocate_throw(true);
        try {
            vec.push_back(constructor_stub(SPECIAL_VALUE));
            FAIL();
        } catch (std::bad_alloc const&) {
            tracking_allocator<constructor_stub>::set_allocate_throw(false);
        }
        test_vec_std_vec_operability(vec, std_vec);
    }

    TEST_F(vector_test, push_back_constructor_throw_test) {
        vector_type vec;
        std::vector<constructor_stub> std_vec = set_up_for_exception(vec);
        tracking_allocator<constructor_stub>::set_construct_throw_count_down(0);
        try {
            vec.push_back(vec.back());
            FAIL();
        } catch (const std::bad_function_call&) {
            tracking_allocator<constructor_stub>::set_construct_throw_count_down(1 << 30);
        }
        test_vec_std_vec_operability(vec, std_vec);
    }

    TEST_F(vector_test, push_back_rvalue_basic_test) {
        vector_type vec;
        constructor_stub stub;
        this -> tracker.mark();
        vec.push_back(std::move(stub));
        this -> tracker.check_marked();
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(vec.size(), 1);
    }

    TEST_F(vector_test, push_back_primitive_stress_test) {
        vector<int> vec;
        for (int i = 0; i < LIMIT; ++i) {
            vec.push_back(i);
        }
        for (int i = 0; i < LIMIT; ++i) {
            EXPECT_EQ(vec[i], i);
        }
    }

    TEST_F(vector_test, push_back_stress_test) {
        vector_type vec;
        for (int i = 0; i < LIMIT; ++i) {
            vec.push_back(constructor_stub(i));
        }
        for (int i = 0; i < LIMIT; ++i) {
            EXPECT_EQ(vec[i].id, i);
        }
    }


    TEST_F(vector_test, pop_back_basic_test) {
        vector_type vec;
        vec.push_back(constructor_stub());
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 1);
        this -> tracker.mark();
        vec.pop_back();
        this -> tracker.check_marked();
        // Once when the temporary constructor_stub goes out of scope
        // Once in pop_back()
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 2);
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, pop_back_stress_test) {
        vector_type vec;
        for (int i = 0; i < LIMIT; ++i) {
            vec.push_back(constructor_stub());
        }
        for (int i = 0; i < LIMIT; ++i) {
            vec.pop_back();
        }
        
    }

    TEST_F(vector_test, push_pop_stress_test) {
        vector_type vec;
        std::vector<constructor_stub> vec_copy;
        int rep = 20;
        int limit = 1000;
        for (int i = 0; i < rep; ++i) {
            for (int j = 0; j < limit; ++j) {
                constructor_stub constructor_stub;
                vec.push_back(constructor_stub);
                vec_copy.push_back(constructor_stub);
            }
            std::size_t half_size = vec.size() / 2;
            for (std::size_t j = 0; j < half_size; ++j) {
                vec.pop_back();
                vec_copy.pop_back();
            }
            test_vec_std_vec_equality(vec, vec_copy);
        }
        while (!vec.empty()) {
            vec.pop_back();
            vec_copy.pop_back();
        }
        EXPECT_EQ(vec.size(), 0);
        
    }

    TEST_F(vector_test, empty_test) {
        vector_type vec;
        EXPECT_TRUE(vec.empty());
        vec.push_back(constructor_stub());
        EXPECT_FALSE(vec.empty());
        vec.pop_back();
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, size_test) {
        vector_type vec;
        for (int i = 0; i < LIMIT; ++i) {
            vec.push_back(constructor_stub());
            EXPECT_EQ(vec.size(), i + 1);
        }
        for (int i = 0; i < LIMIT; ++i) {
            vec.pop_back();
            EXPECT_EQ(vec.size(), LIMIT - i - 1);
        }
    }

    TEST_F(vector_test, reserve_noop_test) {
        vector_type vec(MEDIUM_LIMIT);
        int allocation_count = allocated<constructor_stub>;
        int old_capacity = vec.capacity();
        vec.reserve(MEDIUM_LIMIT / 2);
        EXPECT_EQ(vec.capacity(), old_capacity);
        EXPECT_EQ(vec.size(), MEDIUM_LIMIT);
        EXPECT_EQ(allocation_count, allocated<constructor_stub>);
    }

    TEST_F(vector_test, reserve_reallocate_test) {
        vector_type vec(SMALL_LIMIT);
        int allocation_count = allocated<constructor_stub>;
        int new_capacity = vec.capacity() * 2;
        vec.reserve(vec.capacity() * 2);
        EXPECT_EQ(vec.capacity(), new_capacity);
        EXPECT_EQ(vec.size(), SMALL_LIMIT);
        EXPECT_EQ(allocation_count + new_capacity, allocated<constructor_stub>);
    }

    TEST_F(vector_test, shrink_to_fit_noop_test) {
        vector_type vec;
        vec.reserve(MEDIUM_LIMIT);
        vec.resize(MEDIUM_LIMIT);
        int allocation_count = allocated<constructor_stub>;
        vec.shrink_to_fit();
        EXPECT_EQ(vec.capacity(), MEDIUM_LIMIT);
        EXPECT_EQ(vec.size(), MEDIUM_LIMIT);
        EXPECT_EQ(allocation_count, allocated<constructor_stub>);
    }

    TEST_F(vector_test, shrink_to_fit_reallocate_test) {
        vector_type vec;
        vec.resize(MEDIUM_LIMIT);
        int allocation_count = allocated<constructor_stub>;
        EXPECT_GE(vec.capacity(), MEDIUM_LIMIT);
        this -> tracker.mark();
        vec.shrink_to_fit();
        this -> tracker.check_marked();
        EXPECT_EQ(vec.capacity(), MEDIUM_LIMIT);
        EXPECT_EQ(vec.size(), MEDIUM_LIMIT);
        EXPECT_EQ(allocation_count + MEDIUM_LIMIT, allocated<constructor_stub>);
    }

    TEST_F(vector_test, shrink_to_fit_empty_test) {
        vector_type vec;
        vec.resize(MEDIUM_LIMIT);
        vec.resize(0);
        std::size_t constructor_count = constructor_stub::constructor_invocation_count;
        this -> tracker.mark();
        vec.shrink_to_fit();
        this -> tracker.check_marked();
        EXPECT_EQ(vec.capacity(), vec.__get_default_capacity());
        EXPECT_EQ(vec.size(), 0);
        EXPECT_EQ(constructor_count, constructor_stub::constructor_invocation_count);
    }

    TEST_F(vector_test, front_test) {
        vector_type vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(constructor_stub(i));
        }
        EXPECT_EQ(vec.front().id, 0);
    }

    TEST_F(vector_test, back_test) {
        vector_type vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(constructor_stub(i));
        }
        EXPECT_EQ(vec.back().id, SMALL_LIMIT - 1);
    }

    TEST_F(vector_test, front_alias_test) {
        vector_type vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(constructor_stub(i));
        }
        vec.front().id = SPECIAL_VALUE;
        EXPECT_EQ(vec.front().id, SPECIAL_VALUE);
    }

    TEST_F(vector_test, back_alias_test) {
        vector_type vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(constructor_stub(i));
        }
        vec.back().id = SPECIAL_VALUE;
        EXPECT_EQ(vec.back().id, SPECIAL_VALUE);
    }

    TEST_F(vector_test, begin_end_test) {
        vector<int> vec;
        std::vector<int> src;
        src.resize(BIG_PRIME);
        // Multiples of coprime numbers should complete the congruence class of each
        for (int i = 0, j = 0; i < BIG_PRIME; ++i, j = (j + SMALL_PRIME) % BIG_PRIME) {
            src[j] = i;
        }
        while(!src.empty()) {
            vec.push_back(src.back());
            src.pop_back();
        }
        std::sort(vec.begin(), vec.end());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            EXPECT_EQ(vec[i], i);
        }
    }

    TEST_F(vector_test, cbegin_cend_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(i);
        }
        auto const_it = vec.cbegin();
        for (int i = 0; const_it != vec.cend(); ++i, ++const_it) {
            EXPECT_EQ(i, *const_it);
        }
    }

    TEST_F(vector_test, rbegin_rend_test) {
        vector<int> vec;
        std::vector<int> src;
        src.resize(BIG_PRIME);
        // Multiples of coprime numbers should complete the congruence class of each
        for (int i = 0, j = 0; i < BIG_PRIME; ++i, j = (j + SMALL_PRIME) % BIG_PRIME) {
            src[j] = i;
        }
        while(!src.empty()) {
            vec.push_back(src.back());
            src.pop_back();
        }
        std::sort(vec.rbegin(), vec.rend());
        for (std::size_t i = 0; i < vec.size(); ++i) {
            EXPECT_EQ(vec[i], vec.size() - i - 1);
        }
    }

    TEST_F(vector_test, crbegin_crend_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(i);
        }
        auto const_it = vec.crbegin();
        for (int i = 0; const_it != vec.crend(); ++i, ++const_it) {
            EXPECT_EQ(SMALL_LIMIT - i - 1, *const_it);
        }
    }

    TEST_F(vector_test, foreach_loop_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(i);
        }
        for (int& num : vec) {
            num += 1;
        }
        int i = 1;
        for (int& num : vec) {
            EXPECT_EQ(num, i);
            ++i;
        }
    }

    TEST_F(vector_test, const_foreach_loop_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(i);
        }
        vector<int> immutable_vec(vec);
        int i = 0;
        for (const int& num : vec) {
            EXPECT_EQ(num, i);
            ++i;
        }
    }

    TEST_F(vector_test, clear_test) {
        vector_type vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(i);
        }
        this -> tracker.mark();
        vec.clear();
        this -> tracker.check_marked();
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, insert_single_return_value_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.push_back(i);
        }
        vector<int>::iterator it = vec.insert(vec.begin(), SPECIAL_VALUE);
        EXPECT_EQ(*it, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin());
    }

    template<typename T>
    void insert_single_basic_test(construction_destruction_tracker& tracker) {
        vector<T, tracking_allocator<T> > vec;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec.emplace_back(i);
        }
        T element{SPECIAL_VALUE};
        tracker.mark();
        vec.insert(vec.begin(), element);
        tracker.check_marked();
        EXPECT_EQ(vec[0], SPECIAL_VALUE);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            EXPECT_EQ(vec[i + 1], i);
        }
    }

    TEST_F(vector_test, insert_single_basic_primitive_test) {
        insert_single_basic_test<int>(this -> tracker);
    }

    TEST_F(vector_test, insert_single_basic_test) {
        insert_single_basic_test<constructor_stub>(this -> tracker);
    }

    TEST_F(vector_test, insert_single_exception_safety_test) {
        vector_type vec;
        std::vector<constructor_stub> std_vec = set_up_for_exception(vec);
        tracking_allocator<constructor_stub>::set_allocate_throw(true);
        try {
            vec.insert(vec.begin(), constructor_stub(SPECIAL_VALUE));
            FAIL();
        } catch (std::bad_alloc const&) {
            tracking_allocator<constructor_stub>::set_allocate_throw(false);
        }
        test_vec_std_vec_operability(vec, std_vec);
    }

    template<typename T>
    void insert_single_stress_test() {
        vector<T, tracking_allocator<T> > vec;
        std::vector<T> std_vec;
        for (int i = 0; i < MEDIUM_PRIME; ++i) {
            vec.push_back(i);
            std_vec.push_back(i);
        }
        for (int i = 0, j = 0; i < MEDIUM_PRIME; ++i, j = (j + SMALL_PRIME) % MEDIUM_PRIME) {
            vec.insert(vec.begin() + j, SPECIAL_VALUE);
            std_vec.insert(std_vec.begin() + j, SPECIAL_VALUE);
        }
        test_vec_std_vec_equality(vec, std_vec);   
    }

    TEST_F(vector_test, insert_single_primitive_stress_test) {
        insert_single_stress_test<int>();
    }

    TEST_F(vector_test, insert_single_stress_test) {
        insert_single_stress_test<constructor_stub>();
    }

    TEST_F(vector_test, insert_fill_small_batch_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> standard(SMALL_LIMIT);
        for (int i = 0; i < MEDIUM_LIMIT; i += SMALL_LIMIT) {
            vec.insert(vec.begin() + vec.size() / 2, SMALL_LIMIT, constructor_stub(i));
            standard.insert(standard.begin() + standard.size() / 2, SMALL_LIMIT, constructor_stub(i));
            test_vec_std_vec_equality(vec, standard);
        }
    }

    TEST_F(vector_test, insert_fill_exception_safety_test) {
        vector_type vec;
        std::vector<constructor_stub> std_vec = set_up_for_exception(vec);
        tracking_allocator<constructor_stub>::set_allocate_throw(true);
        try {
            vec.insert(vec.begin() + vec.size() / 2, SMALL_LIMIT, constructor_stub(SPECIAL_VALUE));
            FAIL();
        } catch (std::bad_alloc const&) {
            tracking_allocator<constructor_stub>::set_allocate_throw(false);
        }
        test_vec_std_vec_operability(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_forward_iterator_basic_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> src(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t insert_index = SMALL_LIMIT / 2;
        this -> tracker.mark();
        vec.insert(vec.begin() + insert_index, src.begin(), src.end());
        this -> tracker.check_marked();
        std_vec.insert(std_vec.begin() + insert_index, src.begin(), src.end());
        test_vec_std_vec_equality(vec, std_vec);        
    }

    TEST_F(vector_test, insert_range_forward_iterator_exception_safety_test) {
        vector_type vec;
        std::vector<constructor_stub> std_vec = set_up_for_exception(vec);
        tracking_allocator<constructor_stub>::set_allocate_throw(true);
        try {
            vec.insert(vec.begin() + vec.size() / 2, std_vec.begin(), std_vec.end());
            FAIL();
        } catch (std::bad_alloc const&) {
            tracking_allocator<constructor_stub>::set_allocate_throw(false);
        }
        test_vec_std_vec_operability(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_forward_move_iterator_basic_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> src(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t insert_index = SMALL_LIMIT / 2;
        std_vec.insert(std_vec.begin() + insert_index, src.begin(), src.end());
        this -> tracker.mark();
        int copy_constructor_count = constructor_stub::copy_constructor_invocation_count;
        int assignment_count = constructor_stub::assignment_operator_invocation_count;
        vec.insert(vec.begin() + insert_index, std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
        EXPECT_EQ(copy_constructor_count, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(assignment_count, constructor_stub::assignment_operator_invocation_count);
        this -> tracker.check_marked();
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_forward_iterator_return_value_test) {
        vector<int> vec(SMALL_LIMIT);
        std::vector<int> src(SMALL_LIMIT);
        src[0] = SPECIAL_VALUE;
        size_t insert_index = SMALL_LIMIT / 2;
        vector<int>::iterator it = vec.insert(vec.begin() + insert_index,
            src.begin(), src.end());
        EXPECT_EQ(*it, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin() + insert_index);
    }

    template<typename T>
    void insert_range_forward_iterator_stress_test() {
        vector<T, tracking_allocator<T> > vec(SMALL_LIMIT);
        std::vector<T> src;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            src.emplace_back(SPECIAL_VALUE + i);
        }
        std::vector<T> std_vec(vec.begin(), vec.end());
        for (int i = 0, pos = 0; i < MEDIUM_LIMIT; ++i, pos = (pos + SMALL_PRIME) % vec.size()) {
            vec.insert(vec.begin() + pos, src.begin(), src.end());
            std_vec.insert(std_vec.begin() + pos, src.begin(), src.end());
        }
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_forward_iterator_primitive_stress_test) {
        insert_range_forward_iterator_stress_test<int>();
    }

    TEST_F(vector_test, insert_range_forward_iterator_stress_test) {
        insert_range_forward_iterator_stress_test<constructor_stub>();
    }

    TEST_F(vector_test, insert_range_input_iterator_basic_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t insert_index = SMALL_LIMIT / 2;
        vec.insert(vec.begin() + insert_index, stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
        std_vec.insert(std_vec.begin() + insert_index, stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_input_iterator_exception_safety_test) {
        vector_type vec;
        std::vector<constructor_stub> std_vec = set_up_for_exception(vec);
        tracking_allocator<constructor_stub>::set_allocate_throw(true);
        try {
            vec.insert(vec.begin() + vec.size() / 2, stub_iterator<constructor_stub>(SPECIAL_VALUE),
                stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
            FAIL();
        } catch (std::bad_alloc const&) {
            tracking_allocator<constructor_stub>::set_allocate_throw(false);
        }
        test_vec_std_vec_operability(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_input_iterator_return_value_test) {
        vector_type vec(SMALL_LIMIT);
        size_t insert_index = SMALL_LIMIT / 2;
        vector_type::iterator it = vec.insert(vec.begin() + insert_index,
            stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + 1));
        EXPECT_EQ(it -> id, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin() + insert_index);
    }

    TEST_F(vector_test, insert_range_input_iterator_stress_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        for (int i = 0, pos = 0; i < MEDIUM_LIMIT; ++i, pos = (pos + SMALL_PRIME) % vec.size()) {
            vec.insert(vec.begin() + pos, stub_iterator<constructor_stub>(SPECIAL_VALUE),
                stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
            std_vec.insert(std_vec.begin() + pos, stub_iterator<constructor_stub>(SPECIAL_VALUE),
                stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
        }
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, erase_single_basic_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        int idx = SMALL_LIMIT / 2;
        vec.erase(vec.begin() + idx);
        std_vec.erase(std_vec.begin() + idx);
        test_vec_std_vec_equality(vec, std_vec);        
    }

    TEST_F(vector_test, erase_single_return_value_test) {
        vector_type vec(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            vec[i].id = i;
        }
        int idx = SMALL_LIMIT / 2;
        int expected_id = vec[idx + 1].id;
        vector_type::iterator it = vec.erase(vec.begin() + idx);
        EXPECT_EQ(it -> id, expected_id);
    }

    TEST_F(vector_test, erase_single_stress_test) {
        vector<int, tracking_allocator<int> > vec;
        std::vector<int> std_vec;
        for (int i = 0; i < MEDIUM_PRIME; ++i) {
            vec.push_back(i);
            std_vec.push_back(i);
        }
        int left_size = MEDIUM_LIMIT / 2;
        for (int i = 0, j = 0; i < left_size; ++i, j = (j + SMALL_PRIME) % std_vec.size()) {
            vec.erase(vec.begin() + j);
            std_vec.erase(std_vec.begin() + j);
        }
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, erase_range_basic_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t erase_start = SMALL_LIMIT / 4;
        size_t erase_end = SMALL_LIMIT - erase_start;
        this -> tracker.mark();
        vec.erase(vec.begin() + erase_start, vec.begin() + erase_end);
        this -> tracker.check_marked();
        std_vec.erase(std_vec.begin() + erase_start, std_vec.begin() + erase_end);
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, erase_range_return_value_test) {
        vector<int> vec(SMALL_LIMIT);
        size_t erase_start = SMALL_LIMIT / 4;
        size_t erase_end = SMALL_LIMIT - erase_start;
        vec[erase_end] = SPECIAL_VALUE;
        vector<int>::iterator it =
            vec.erase(vec.begin() + erase_start, vec.begin() + erase_end);
        EXPECT_EQ(*it, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin() + erase_start);
    }

    template<typename T>
    void erase_range_stress_test() {
        vector<T, tracking_allocator<T> > vec;
        for (std::size_t i = 0; i < MEDIUM_LIMIT; ++i) {
            vec.emplace_back(i);
        }
        std::vector<T> std_vec(vec.begin(), vec.end());
        std::size_t idx = 0;
        while (!std_vec.empty()) {
            std::size_t removed_count = std::min<int>(std_vec.size(), SMALL_LIMIT);
            if (idx + removed_count >= vec.size()) {
                idx = 0;
            }
            vec.erase(vec.begin() + idx, vec.begin() + idx + removed_count);
            std_vec.erase(std_vec.begin() + idx, std_vec.begin() + idx + removed_count);
            test_vec_std_vec_equality(vec, std_vec);
            idx = (idx + SMALL_PRIME) % (std_vec.size() + 1);
        }
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, erase_range_primitive_stress_test) {
        erase_range_stress_test<int>();
    }

    TEST_F(vector_test, erase_range_stress_test) {
        erase_range_stress_test<constructor_stub>();
    }

    TEST_F(vector_test, emplace_back_forward_test) {
        vector<forward_stub> vec;
        vec.emplace_back(SPECIAL_VALUE);
        EXPECT_EQ(vec.back().construct_type,
            forward_stub::constructor_type::int_constructor);
        EXPECT_EQ(vec.back().n, SPECIAL_VALUE);
        vec.emplace_back(0.1f);
        EXPECT_EQ(vec.back().construct_type,
            forward_stub::constructor_type::float_constructor);
        vec.emplace_back(std::string());
        EXPECT_EQ(vec.back().construct_type,
            forward_stub::constructor_type::string_constructor);
    }

    TEST_F(vector_test, emplace_back_return_value_test) {
        vector<forward_stub> vec;
        forward_stub& stub = vec.emplace_back(SPECIAL_VALUE);
        EXPECT_EQ(vec[0].n, SPECIAL_VALUE);
        stub.n++;
        EXPECT_EQ(vec[0].n, SPECIAL_VALUE + 1);
    }

    TEST_F(vector_test, resize_append_test) {
        vector_type vec;
        this -> tracker.mark();
        vec.resize(SMALL_LIMIT);
        this -> tracker.check_marked();
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(vec.size(), SMALL_LIMIT);
    }

    TEST_F(vector_test, resize_shrink_test) {
        vector_type vec(SMALL_LIMIT * 2);
        this -> tracker.mark();
        vec.resize(SMALL_LIMIT);
        this -> tracker.check_marked();
        EXPECT_EQ(constructor_stub::destructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(vec.size(), SMALL_LIMIT);
    }

    TEST_F(vector_test, resize_value_test) {
        vector_type vec;
        constructor_stub stub(SPECIAL_VALUE);
        this -> tracker.mark();
        vec.resize(SMALL_LIMIT, stub);
        for (constructor_stub& stub : vec) {
            EXPECT_EQ(stub.id, SPECIAL_VALUE);
        }
        this -> tracker.check_marked();
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(vec.size(), SMALL_LIMIT);
    }

    TEST_F(vector_test, copy_only_constructor_test) {
        using copy_only_vector_type = vector<copy_only_constructor_stub, tracking_allocator<copy_only_constructor_stub>>;
        copy_only_vector_type vec1;
        copy_only_vector_type vec2(SMALL_LIMIT);
        copy_only_constructor_stub stub;
        copy_only_vector_type vec3(SMALL_LIMIT, stub);
        copy_only_vector_type vec4(vec3.begin(), vec3.end());
        copy_only_vector_type vec5(vec4);
        copy_only_vector_type vec6(std::move(vec5));
    }

    TEST_F(vector_test, copy_only_operation_test) {
        vector<copy_only_constructor_stub, tracking_allocator<copy_only_constructor_stub> > vec;
        std::vector<constructor_stub> standard;
        vec.insert(vec.begin(), copy_only_constructor_stub(SPECIAL_VALUE));
        standard.insert(standard.begin(), constructor_stub(SPECIAL_VALUE));
        copy_only_constructor_stub stub(SPECIAL_VALUE2);
        constructor_stub mirror(SPECIAL_VALUE2);
        vec.push_back(stub);
        standard.push_back(mirror);
        vec.push_back(copy_only_constructor_stub(stub));
        standard.push_back(constructor_stub(mirror));
        vec.emplace_back(stub);
        standard.emplace_back(mirror);
        for (std::size_t i = 0; i < MEDIUM_LIMIT; ++i) {
            vec.push_back(random_number());
            standard.push_back(vec.back().id);
        }
        std::vector<int> nums(SMALL_LIMIT, SPECIAL_VALUE);

        standard.insert(standard.begin() + standard.size() / 2, nums.begin(), nums.end());
        standard.insert(standard.begin() + standard.size() / 2, nums.begin(), nums.end());
        standard.insert(standard.begin() + standard.size() / 2, SMALL_LIMIT, mirror);
        // If the iterator's value type is not the same as the vector value type, we don't care whether the vector value type
        // has a valid copy or move constructor
        vec.insert(vec.begin() + vec.size() / 2, nums.begin(), nums.end());
        vec.insert(vec.begin() + vec.size() / 2, std::make_move_iterator(nums.begin()), std::make_move_iterator(nums.end()));
        vec.insert(vec.begin() + vec.size() / 2, SMALL_LIMIT, stub);
        EXPECT_EQ(standard.size(), vec.size());
        for (std::size_t i = 0; i < standard.size(); ++i) {
            EXPECT_EQ(vec[i].id, standard[i].id);
        }
        vec.erase(vec.begin() + vec.size() / 3, vec.end());
        vec.pop_back();
    }

    TEST_F(vector_test, move_only_constructor_test) {
        using move_only_vector_type = vector<move_only_constructor_stub, tracking_allocator<move_only_constructor_stub>>;
        move_only_vector_type vec1;
        move_only_vector_type vec2(SMALL_LIMIT);
        move_only_vector_type vec3(std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
        move_only_vector_type vec4(std::move(vec3));
        EXPECT_TRUE(vec3.empty());
    }

    TEST_F(vector_test, move_only_operation_test) {
        vector<move_only_constructor_stub, tracking_allocator<move_only_constructor_stub>> vec;
        std::vector<constructor_stub> standard;
        vec.insert(vec.begin(), move_only_constructor_stub(SPECIAL_VALUE));
        standard.insert(standard.begin(), constructor_stub(SPECIAL_VALUE));
        vec.push_back(move_only_constructor_stub(SPECIAL_VALUE2));
        standard.push_back(constructor_stub(SPECIAL_VALUE2));
        vec.emplace_back(move_only_constructor_stub(SPECIAL_VALUE2));
        standard.emplace_back(constructor_stub(SPECIAL_VALUE2));
        for (std::size_t i = 0; i < MEDIUM_LIMIT; ++i) {
            vec.push_back(random_number());
            standard.push_back(vec.back().id);
        }
        std::vector<int> nums(SMALL_LIMIT, SPECIAL_VALUE);
        vector<move_only_constructor_stub, tracking_allocator<move_only_constructor_stub> > src(SMALL_LIMIT);
        std::vector<constructor_stub> standard_src(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            src[i] = i;
            standard_src[i] = i;
        }

        standard.insert(standard.begin() + standard.size() / 2, nums.begin(), nums.end());
        standard.insert(standard.begin() + standard.size() / 2, nums.begin(), nums.end());
        standard.insert(standard.begin() + standard.size() / 2, standard_src.begin(), standard_src.end());
        // If the iterator's value type is not the same as the vector value type, we don't care whether the vector value type
        // has a valid copy or move constructor
        vec.insert(vec.begin() + vec.size() / 2, nums.begin(), nums.end());
        vec.insert(vec.begin() + vec.size() / 2, std::make_move_iterator(nums.begin()), std::make_move_iterator(nums.end()));
        vec.insert(vec.begin() + vec.size() / 2, std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
        EXPECT_EQ(standard.size(), vec.size());
        for (std::size_t i = 0; i < standard.size(); ++i) {
            EXPECT_EQ(vec[i].id, standard[i].id);
        }
        vec.erase(vec.begin() + vec.size() / 3, vec.end());
        vec.pop_back();
    }

    TEST_F(vector_test, swap_test) {
        vector_type vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        vector_type vec2(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec2(vec2.begin(), vec2.end());
        vec.swap(vec2);
        test_vec_std_vec_equality(vec, std_vec2);
        test_vec_std_vec_equality(vec2, std_vec);
    }

    vector_type to_stub_vector(vector<int> vec) {
        vector_type stubs;
        for (int num : vec) {
            stubs.emplace_back(num);
        }
        return stubs;
    }

    TEST_F(vector_test, equality_test) {
        vector_type vec1 = to_stub_vector({0, 1, 2});
        vector_type vec2 = to_stub_vector({0, 1, 2});
        EXPECT_EQ(vec1, vec2);
    }

    TEST_F(vector_test, inequality_test) {
        vector_type vec1 = to_stub_vector({0, 1, 2});
        vector_type vec2 = to_stub_vector({0, 1, 3});
        EXPECT_NE(vec1, vec2);
    }

    TEST_F(vector_test, three_way_comparison_test) {
        vector_type vec1 = to_stub_vector({0, 1, 2});
        vector_type vec2 = to_stub_vector({0, 1, 3});
        EXPECT_LT(vec1, vec2);
        EXPECT_LE(vec1, vec2);
        EXPECT_GT(vec2, vec1);
        EXPECT_GE(vec2, vec1);
    }

    TEST_F(vector_test, three_way_comparison_length_test) {
        vector_type vec1 = to_stub_vector({0, 1});
        vector_type vec2 = to_stub_vector({0, 1, 3});
        EXPECT_LT(vec1, vec2);
        EXPECT_LE(vec1, vec2);
        EXPECT_GT(vec2, vec1);
        EXPECT_GE(vec2, vec1);
    }
}
