#include "gtest/gtest.h"
#include "deque/deque.h"
#include <deque>
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/utility/tracking_allocator.h"
#include "tst/utility/common.h"

namespace {
    using namespace algo;
    class deque_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            tracking_allocator<constructor_stub>::reset();
            tracking_allocator<constructor_stub*>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<constructor_stub>::check();
            tracking_allocator<constructor_stub*>::check();
        }
    };

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SPECIAL_VALUE2 = 0xbeefbabe;
    static const int BIG_PRIME = 7759;
    static const int MEDIUM_PRIME = 443;
    static const int SMALL_PRIME = 19;
    static const int LIMIT = 10000;
    static const int MEDIUM_LIMIT = 1000;
    static const int SMALL_LIMIT = 10;
    static tracking_allocator<constructor_stub> allocator;
    using deque_type = deque<constructor_stub, tracking_allocator<constructor_stub> >;

    template<typename T>
    void test_deq_std_deq_equality(deque<T, tracking_allocator<T> >& deq, std::deque<T>& std_deq) {
        EXPECT_EQ(deq.size(), std_deq.size());
        for (std::size_t i = 0; i < deq.size(); ++i) {
            EXPECT_EQ(deq[i], std_deq[i]);
        }
    }

    template<typename T>
    void test_deq_std_deq_equality_iterator(deque<T>& deq, std::deque<T>& std_deq) {
        EXPECT_EQ(deq.size(), std_deq.size());
        auto standard_it = std_deq.begin();
        for (auto it = deq.begin(); it != deq.end(); ++it, ++standard_it) {
            EXPECT_EQ(*standard_it, *it);
        }
    }

    void default_constructor_test_helper(deque_type& deq) {
        EXPECT_TRUE(deq.empty());
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(DEFAULT_NUM_CHUNKS + 2, allocated<constructor_stub*>);
        EXPECT_EQ(0, allocated<constructor_stub>);
    }

    void is_allocator_correct(deque_type& deq) {
        EXPECT_EQ(deq.get_allocator(), allocator);
        EXPECT_EQ(deq.__get_pointer_allocator().id, allocator.id);
    }

    TEST_F(deque_test, trait_test) {
        bool correct = std::is_same_v<typename deque_type::value_type, constructor_stub>
            && std::is_same_v<typename deque_type::allocator_type, tracking_allocator<constructor_stub> >
            && std::is_same_v<typename deque_type::size_type, std::size_t>
            && std::is_same_v<typename deque_type::difference_type, std::ptrdiff_t>
            && std::is_same_v<typename deque_type::reference, constructor_stub&>
            && std::is_same_v<typename deque_type::const_reference, const constructor_stub&>
            && std::is_same_v<typename deque_type::pointer, constructor_stub*>
            && std::is_same_v<typename deque_type::const_pointer, const constructor_stub*>
            && std::is_same_v<typename deque_type::iterator, deque_iterator<constructor_stub> >
            && std::is_same_v<typename deque_type::const_iterator, deque_iterator<const constructor_stub>>
            && std::is_same_v<typename deque_type::reverse_iterator, deque_iterator<constructor_stub, true> >
            && std::is_same_v<typename deque_type::const_reverse_iterator, deque_iterator<const constructor_stub, true>  >;
        EXPECT_TRUE(correct);
    }

    TEST_F(deque_test, default_constructor_test) {
        deque_type deq;
        default_constructor_test_helper(deq);
    }
    
    TEST_F(deque_test, allocator_constructor_test) {
        deque_type deq(allocator);
        default_constructor_test_helper(deq);
        is_allocator_correct(deq);
    }

    void default_fill_constructor_test_helper() {
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, MEDIUM_LIMIT);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, MEDIUM_LIMIT);
        std::size_t chunks = DEFAULT_NUM_CHUNKS 
            + (MEDIUM_LIMIT + CHUNK_SIZE<constructor_stub> - 1) / CHUNK_SIZE<constructor_stub>;
        std::size_t allocated_chunks = (MEDIUM_LIMIT + CHUNK_SIZE<constructor_stub> - 1) / CHUNK_SIZE<constructor_stub>;
        EXPECT_EQ(chunks + 2, allocated<constructor_stub*>);
        EXPECT_EQ(allocated_chunks * CHUNK_SIZE<constructor_stub>, allocated<constructor_stub>);
    }

    TEST_F(deque_test, default_fill_constructor_test) {
        deque_type deq(MEDIUM_LIMIT, allocator);
        default_fill_constructor_test_helper();
        is_allocator_correct(deq);
    }

    TEST_F(deque_test, default_fill_constructor_optional_test) {
        deque_type deq(MEDIUM_LIMIT);
        default_fill_constructor_test_helper();
    }

    void fill_constructor_test_helper(deque_type& deq) {
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(MEDIUM_LIMIT, constructor_stub(SPECIAL_VALUE));
        test_deq_std_deq_equality(deq, std_deq);
        std::size_t chunks = DEFAULT_NUM_CHUNKS
            + (MEDIUM_LIMIT + CHUNK_SIZE<constructor_stub> - 1) / CHUNK_SIZE<constructor_stub>;
        std::size_t allocated_chunks = (MEDIUM_LIMIT + CHUNK_SIZE<constructor_stub> - 1) / CHUNK_SIZE<constructor_stub>;
        EXPECT_EQ(chunks + 2, allocated<constructor_stub*>);
        EXPECT_EQ(allocated_chunks * CHUNK_SIZE<constructor_stub>, allocated<constructor_stub>);
    }

    TEST_F(deque_test, fill_constructor_test) {
        deque_type deq(MEDIUM_LIMIT, constructor_stub(SPECIAL_VALUE), allocator);
        fill_constructor_test_helper(deq);
        is_allocator_correct(deq);
    }

    TEST_F(deque_test, fill_constructor_optional_test) {
        deque_type deq(MEDIUM_LIMIT, constructor_stub(SPECIAL_VALUE));
        fill_constructor_test_helper(deq);
    }

    TEST_F(deque_test, range_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        deque_type deq(stubs.begin(), stubs.end(), allocator);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(stubs.begin(), stubs.end());
        test_deq_std_deq_equality(deq, std_deq);
        is_allocator_correct(deq);
    }

    TEST_F(deque_test, range_constructor_optional_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        deque_type deq(stubs.begin(), stubs.end());
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(stubs.begin(), stubs.end());
        test_deq_std_deq_equality(deq, std_deq);
    }

    TEST_F(deque_test, copy_constructor_test) {
        deque_type deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        std::size_t copy_count = constructor_stub::copy_constructor_invocation_count;
        std::size_t chunk_alloc_count = allocated<constructor_stub*>;
        std::size_t stub_alloc_count = allocated<constructor_stub>;
        std::size_t chunks = 2 + deq.__get_num_chunks();
        std::size_t allocated_chunks = deq.__get_num_active_chunks();
        deque_type deq_copy(deq);
        EXPECT_EQ(copy_count + MEDIUM_LIMIT, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(chunk_alloc_count + chunks, allocated<constructor_stub*>);
        EXPECT_EQ(stub_alloc_count + allocated_chunks * CHUNK_SIZE<constructor_stub>, allocated<constructor_stub>);
        test_deq_std_deq_equality(deq_copy, std_deq);
        EXPECT_EQ(deq.get_allocator(), deq_copy.get_allocator());
        EXPECT_EQ(deq.__get_pointer_allocator(), deq_copy.__get_pointer_allocator());
    }

    TEST_F(deque_test, allocator_copy_constructor_test) {
        deque_type deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        std::size_t copy_count = constructor_stub::copy_constructor_invocation_count;
        std::size_t chunk_alloc_count = allocated<constructor_stub*>;
        std::size_t stub_alloc_count = allocated<constructor_stub>;
        std::size_t chunks = 2 + deq.__get_num_chunks();
        std::size_t allocated_chunks = deq.__get_num_active_chunks();
        deque_type deq_copy(deq, allocator);
        EXPECT_EQ(copy_count + MEDIUM_LIMIT, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(chunk_alloc_count + chunks, allocated<constructor_stub*>);
        EXPECT_EQ(stub_alloc_count + allocated_chunks * CHUNK_SIZE<constructor_stub>, allocated<constructor_stub>);
        test_deq_std_deq_equality(deq_copy, std_deq);
        is_allocator_correct(deq_copy);
    }

    TEST_F(deque_test, move_constructor_test) {
        deque_type deq(MEDIUM_LIMIT, allocator);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        std::size_t constructor_count = constructor_stub::constructor_invocation_count;
        std::size_t chunk_alloc_count = allocated<constructor_stub*>;
        std::size_t stub_alloc_count = allocated<constructor_stub>;
        deque_type deq_copy(std::move(deq));
        EXPECT_EQ(constructor_count, constructor_stub::constructor_invocation_count);
        EXPECT_EQ(chunk_alloc_count, allocated<constructor_stub*>);
        EXPECT_EQ(stub_alloc_count, allocated<constructor_stub>);
        test_deq_std_deq_equality(deq_copy, std_deq);
        EXPECT_EQ(allocator, deq_copy.get_allocator());
        EXPECT_EQ(allocator.id, deq_copy.__get_pointer_allocator().id);
    }

    TEST_F(deque_test, allocator_move_constructor_test) {
        deque_type deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        std::size_t constructor_count = constructor_stub::constructor_invocation_count;
        std::size_t chunk_alloc_count = allocated<constructor_stub*>;
        std::size_t stub_alloc_count = allocated<constructor_stub>;
        deque_type deq_copy(std::move(deq), allocator);
        EXPECT_EQ(constructor_count, constructor_stub::constructor_invocation_count);
        EXPECT_EQ(chunk_alloc_count, allocated<constructor_stub*>);
        EXPECT_EQ(stub_alloc_count, allocated<constructor_stub>);
        test_deq_std_deq_equality(deq_copy, std_deq);
        is_allocator_correct(deq_copy);
    }

    TEST_F(deque_test, assignment_operator_test) {
        deque_type deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        deque_type deq_copy;
        deq_copy = deq;
        test_deq_std_deq_equality(deq, std_deq);
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, assignment_operator_shortcut_test) {
        deque<int, tracking_allocator<int> > deq(MEDIUM_LIMIT);
        std::deque<int> std_deq(deq.begin(), deq.end());
        deque<int, tracking_allocator<int> > deq_copy(MEDIUM_LIMIT * 2);
        // No buffer allocated if the destination is big enough
        std::size_t alloc_count = allocated<int>;
        EXPECT_GT(alloc_count, 0);
        deq_copy = deq;
        EXPECT_EQ(alloc_count, allocated<int>);
        test_deq_std_deq_equality(deq, std_deq);
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, move_assignment_operator_test) {
        deque_type deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        deque_type deq_copy;
        deq_copy = std::move(deq);
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, front_test) {
        deque_type deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        EXPECT_EQ(deq.front().id, 0);
    }

    TEST_F(deque_test, back_test) {
        deque_type deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        EXPECT_EQ(deq.back().id, SMALL_LIMIT - 1);
    }

    TEST_F(deque_test, front_alias_test) {
        deque_type deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        deq.front().id = SPECIAL_VALUE;
        EXPECT_EQ(deq.front().id, SPECIAL_VALUE);
    }

    TEST_F(deque_test, back_alias_test) {
        deque_type deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        deq.back().id = SPECIAL_VALUE;
        EXPECT_EQ(deq.back().id, SPECIAL_VALUE);
    }

    TEST_F(deque_test, push_back_copy_test) {
        deque_type deq;
        constructor_stub stub(0);
        deq.push_back(stub);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 2);
    }

    TEST_F(deque_test, push_back_move_test) {
        deque_type deq;
        deq.push_back(constructor_stub(0));
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 2);
    }

    template<typename T>
    void test_deq_std_deq_operability(deque<T, tracking_allocator<T> >& deq, std::deque<T>& standard) {
        EXPECT_TRUE(deq.__is_valid());
        test_deq_std_deq_equality(deq, standard);
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        for (auto& s : stubs) {
            deq.push_back(s);
            standard.push_back(s);
        }
        EXPECT_TRUE(deq.__is_valid());
        test_deq_std_deq_equality(deq, standard);
        for (auto& s : stubs) {
            deq.push_front(s);
            standard.push_front(s);
        }
        EXPECT_TRUE(deq.__is_valid());
        test_deq_std_deq_equality(deq, standard);
        deq.insert(deq.begin() + deq.size() / 2, stubs.begin(), stubs.end());
        standard.insert(standard.begin() + standard.size() / 2, stubs.begin(), stubs.end());
        EXPECT_TRUE(deq.__is_valid());
        test_deq_std_deq_equality(deq, standard);
    }

    std::deque<constructor_stub> set_up_for_back_exception(deque_type& deq, auto func, std::size_t limit = 0) {
        deq.emplace_back(SPECIAL_VALUE);
        std::deque<constructor_stub> standard(1, deq.back());
        while (func(deq) > limit) {
            deq.push_back(constructor_stub(SPECIAL_VALUE + func(deq)));
            standard.push_back(deq.back());
        }
        return standard;
    }
    
    template<typename T>
    void push_back_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_back_exception(deq, func);
        tracking_allocator<T>::set_allocate_throw(true);
        try {
            deq.push_back(deq.back());
            FAIL();
        } catch (const std::bad_alloc&) {
            tracking_allocator<T>::set_allocate_throw(false);
        }
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, push_back_exception_safety_new_chunk_test) {
        push_back_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__back_capacity(); });
    }

    TEST_F(deque_test, push_back_exception_safety_new_map_test) {
        push_back_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__back_ghost_capacity(); });
    }

    TEST_F(deque_test, emplace_back_test) {
        deque_type deq;
        deq.emplace_back(0);
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 1);
    }

    TEST_F(deque_test, push_front_copy_test) {
        deque_type deq;
        constructor_stub stub(0);
        deq.push_front(stub);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 2);
    }

    TEST_F(deque_test, push_front_move_test) {
        deque_type deq;
        deq.push_front(constructor_stub(0));
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 2);
    }

    std::deque<constructor_stub> set_up_for_front_exception(deque_type& deq, auto func, std::size_t limit = 0) {
        deq.emplace_front(SPECIAL_VALUE);
        std::deque<constructor_stub> standard(1, deq.back());
        while (func(deq) > limit) {
            deq.push_front(constructor_stub(SPECIAL_VALUE + func(deq)));
            standard.push_front(deq.front());
        }
        return standard;
    }

    template<typename T>
    void push_front_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_front_exception(deq, func);
        tracking_allocator<T>::set_allocate_throw(true);
        try {
            deq.push_front(deq.back());
            FAIL();
        } catch (const std::bad_alloc&) {
            tracking_allocator<T>::set_allocate_throw(false);
        }
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, push_front_exception_safety_new_chunk_test) {
        push_front_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__front_capacity(); });
    }

    TEST_F(deque_test, push_front_exception_safety_new_map_test) {
        push_front_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__front_ghost_capacity(); });
    }

    TEST_F(deque_test, emplace_front_test) {
        deque_type deq;
        deq.emplace_front(0);
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 1);
    }

    TEST_F(deque_test, shift_left_test) {
        deque_type deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
            deq.pop_back();
            EXPECT_EQ(deq.__get_num_active_chunks(), 0);
            EXPECT_LE(deq.__get_num_chunks(), 3);
        }
    }

    TEST_F(deque_test, shift_left_gap_test) {
        deque_type deq;
        deq.push_front(constructor_stub(0));
        for (int i = 1; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
            deq.pop_back();
            EXPECT_EQ(deq.__get_num_active_chunks(), 1);
            EXPECT_LE(deq.__get_num_chunks(), 6);
        }
    }

    TEST_F(deque_test, shift_right_test) {
        deque_type deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
            deq.pop_front();
            EXPECT_EQ(deq.__get_num_active_chunks(), 0);
            EXPECT_LE(deq.__get_num_chunks(), 3);
        }
    }

    TEST_F(deque_test, shift_right_gap_test) {
        deque_type deq;
        deq.push_back(constructor_stub(0));
        for (int i = 1; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
            deq.pop_front();
            EXPECT_EQ(deq.__get_num_active_chunks(), 1);
            EXPECT_LE(deq.__get_num_chunks(), 6);
        }
    }

    void push_pop_back_tester(std::size_t limit) {
        deque_type deq;
        for (std::size_t i = 0; i < limit; ++i) {
            deq.push_back(constructor_stub(i));
        }
        for (std::size_t i = 0; i < limit; ++i) {
            EXPECT_EQ(deq[i].id, i);
        }
        for (std::size_t i = 0; i < limit; ++i) {
            deq.pop_back();
        }
    }

    TEST_F(deque_test, push_pop_back_test) {
        push_pop_back_tester(SMALL_LIMIT);
    }

    TEST_F(deque_test, push_pop_back_intermediate_test) {
        push_pop_back_tester(MEDIUM_LIMIT);
    }

    TEST_F(deque_test, push_pop_back_stress_test) {
        push_pop_back_tester(LIMIT);
    }

    TEST_F(deque_test, push_pop_back_mixed_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_back();
                standard.pop_back();
            }
        }
    }

    void push_pop_front_tester(std::size_t limit) {
        deque_type deq;
        for (std::size_t i = 0; i < limit; ++i) {
            deq.push_front(constructor_stub(i));
        }
        for (std::size_t i = 0; i < limit; ++i) {
            EXPECT_EQ(limit - 1 - deq[i].id, i);
        }
        for (std::size_t i = 0; i < limit; ++i) {
            deq.pop_front();
        }
    }

    TEST_F(deque_test, push_pop_front_test) {
        push_pop_front_tester(SMALL_LIMIT);
    }
    
    TEST_F(deque_test, push_pop_front_intermediate_test) {
        push_pop_front_tester(MEDIUM_LIMIT);
    }

    TEST_F(deque_test, push_pop_front_stress_test) {
        push_pop_front_tester(LIMIT);
    }

    TEST_F(deque_test, push_pop_front_mixed_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_front();
                standard.pop_front();
            }
        }
    }

    TEST_F(deque_test, all_push_pop_mixed_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_back();
                standard.pop_back();
            }
            EXPECT_EQ(0, deq.__get_num_active_chunks());
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_front();
                standard.pop_front();
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_single_move_test) {
        deque_type deq;
        deq.insert(deq.begin(), constructor_stub(0));
        EXPECT_EQ(1, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(2, constructor_stub::constructor_invocation_count);
    }

    TEST_F(deque_test, insert_emplace_test) {
        deque_type deq;
        deq.emplace(deq.begin(), 0);
        EXPECT_EQ(1, constructor_stub::id_constructor_invocation_count);
        EXPECT_EQ(1, constructor_stub::constructor_invocation_count);
    }

    TEST_F(deque_test, insert_single_begin_test) {
        deque_type deq;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.begin(), constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            EXPECT_EQ(MEDIUM_LIMIT - deq[i].id - 1, i);
        }
    }

    TEST_F(deque_test, insert_single_end_test) {
        deque_type deq;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.end(), constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            EXPECT_EQ(deq[i].id, i);
        }
    }

    TEST_F(deque_test, insert_single_left_test) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.begin() + j, constructor_stub(i));
            standard.insert(standard.begin() + j, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    template<typename T>
    void insert_single_with_exception(deque_type& deq, std::size_t offset) {
        tracking_allocator<T>::set_allocate_throw(true);
        try {
            deq.insert(deq.begin() + offset, constructor_stub(SPECIAL_VALUE));
            FAIL();
        } catch (const std::bad_alloc&) {
            tracking_allocator<T>::set_allocate_throw(false);
        }
    }

    template<typename T>
    void insert_single_left_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_front_exception(deq, func);
        insert_single_with_exception<T>(deq, deq.size() / 3);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_single_left_exception_safety_new_chunk_test) {
        insert_single_left_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__front_capacity(); });
    }

    TEST_F(deque_test, insert_single_left_exception_safety_new_map_test) {
        insert_single_left_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__front_ghost_capacity(); });
    }

    TEST_F(deque_test, insert_single_right_test) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.begin() + j, constructor_stub(i));
            standard.insert(standard.begin() + j, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    template<typename T>
    void insert_single_right_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_back_exception(deq, func);
        insert_single_with_exception<T>(deq, deq.size() * 2 / 3);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_single_right_exception_safety_new_chunk_test) {
        insert_single_right_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__back_capacity(); });
    }

    TEST_F(deque_test, insert_single_right_exception_safety_new_map_test) {
        insert_single_right_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__back_ghost_capacity(); });
    }

    void insert_fill_begin_tester(std::size_t batch) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin(), batch, constructor_stub(i));
            standard.insert(standard.begin(), batch, constructor_stub(i));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_begin_small_batch_test) {
        insert_fill_begin_tester(5);
    }

    TEST_F(deque_test, insert_fill_begin_big_batch_test) {
        insert_fill_begin_tester(200);
    }

    template<typename T>
    void insert_fill_with_exception(deque_type& deq, std::size_t offset, std::size_t repeat) {
        tracking_allocator<T>::set_allocate_throw(true);
        try {
            deq.insert(deq.begin() + offset, repeat, constructor_stub(SPECIAL_VALUE));
            FAIL();
        } catch (const std::bad_alloc&) {
            tracking_allocator<T>::set_allocate_throw(false);
        }
    }

    template<typename T>
    void insert_fill_begin_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_front_exception(deq, func, SMALL_LIMIT / 2);
        insert_fill_with_exception<T>(deq, 0, SMALL_LIMIT);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_fill_begin_exception_safety_new_chunk_test) {
        insert_fill_begin_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__front_capacity(); });
    }

    TEST_F(deque_test, insert_fill_begin_exception_safety_new_map_test) {
        insert_fill_begin_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__front_ghost_capacity(); });
    }

    void insert_fill_end_tester(std::size_t batch) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.end(), batch, constructor_stub(i));
            standard.insert(standard.end(), batch, constructor_stub(i));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_end_small_batch_test) {
        insert_fill_end_tester(5);
    }

    TEST_F(deque_test, insert_fill_end_big_batch_test) {
        insert_fill_end_tester(200);
    }

    template<typename T>
    void insert_fill_end_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_back_exception(deq, func, SMALL_LIMIT / 2);
        insert_fill_with_exception<T>(deq, deq.size(), SMALL_LIMIT);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_fill_end_exception_safety_new_chunk_test) {
        insert_fill_end_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__back_capacity(); });
    }

    TEST_F(deque_test, insert_fill_end_exception_safety_new_map_test) {
        insert_fill_end_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__back_ghost_capacity(); });
    }

    void insert_fill_left_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, batch, constructor_stub(i));
            standard.insert(standard.begin() + j, batch, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_left_small_batch_test) {
        insert_fill_left_tester(5);
    }

    TEST_F(deque_test, insert_fill_left_big_batch_test) {
        insert_fill_left_tester(200);
    }

    template<typename T>
    void insert_fill_left_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_front_exception(deq, func, SMALL_LIMIT / 2);
        insert_fill_with_exception<T>(deq, deq.size() / 3, SMALL_LIMIT);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_fill_left_exception_safety_new_chunk_test) {
        insert_fill_left_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__front_capacity(); });
    }

    TEST_F(deque_test, insert_fill_left_exception_safety_new_map_test) {
        insert_fill_left_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__front_ghost_capacity(); });
    }

    void insert_fill_right_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, batch, constructor_stub(i));
            standard.insert(standard.begin() + j, batch, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_right_small_batch_test) {
        insert_fill_right_tester(5);
    }

    TEST_F(deque_test, insert_fill_right_big_batch_test) {
        insert_fill_right_tester(200);
    }

    template<typename T>
    void insert_fill_right_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_back_exception(deq, func, SMALL_LIMIT / 2);
        insert_fill_with_exception<T>(deq, deq.size() * 2 / 3, SMALL_LIMIT);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_fill_right_exception_safety_new_chunk_test) {
        insert_fill_right_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__back_capacity(); });
    }

    TEST_F(deque_test, insert_fill_right_exception_safety_new_map_test) {
        insert_fill_right_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__back_ghost_capacity(); });
    }

    void insert_range_begin_tester(std::size_t batch) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin(), content.begin(), content.end());
            standard.insert(standard.begin(), content.begin(), content.end());
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_begin_small_batch_test) {
        insert_range_begin_tester(5);
    }

    TEST_F(deque_test, insert_range_begin_big_batch_test) {
        insert_range_begin_tester(200);
    }

    void insert_range_end_tester(std::size_t batch) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.end(), content.begin(), content.end());
            standard.insert(standard.end(), content.begin(), content.end());
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_end_small_batch_test) {
        insert_range_end_tester(5);
    }

    TEST_F(deque_test, insert_range_end_big_batch_test) {
        insert_range_end_tester(200);
    }

    void insert_range_left_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, content.begin(), content.end());
            standard.insert(standard.begin() + j, content.begin(), content.end());
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_left_small_batch_test) {
        insert_range_left_tester(5);
    }

    TEST_F(deque_test, insert_range_left_big_batch_test) {
        insert_range_left_tester(200);
    }

    void insert_range_right_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, content.begin(), content.end());
            standard.insert(standard.begin() + j, content.begin(), content.end());
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_right_small_batch_test) {
        insert_range_right_tester(5);
    }

    TEST_F(deque_test, insert_range_right_big_batch_test) {
        insert_range_right_tester(200);
    }

    void insert_range_begin_input_iterator_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.begin(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_begin_input_iterator_small_batch_test) {
        insert_range_begin_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_begin_input_iterator_big_batch_test) {
        insert_range_begin_input_iterator_tester(200);
    }

    template<typename T>
    void insert_range_input_iterator_with_exception(deque_type& deq, std::size_t offset) {
        tracking_allocator<T>::set_allocate_throw(true);
        try {
            deq.insert(deq.begin() + offset, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(MEDIUM_LIMIT));
            FAIL();
        } catch (const std::bad_alloc&) {
            tracking_allocator<T>::set_allocate_throw(false);
        }
    }

    template<typename T>
    void insert_range_input_iterator_begin_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_front_exception(deq, func, SMALL_LIMIT / 2);
        insert_range_input_iterator_with_exception<T>(deq, 0);
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_range_input_iterator_begin_exception_safety_new_chunk_test) {
        insert_range_input_iterator_begin_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__front_capacity(); });
    }

    TEST_F(deque_test, insert_range_input_iterator_begin_exception_safety_new_map_test) {
        insert_range_input_iterator_begin_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__front_ghost_capacity(); });
    }

    void insert_range_left_input_iterator_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_left_input_iterator_small_batch_test) {
        insert_range_left_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_left_input_iterator_big_batch_test) {
        insert_range_left_input_iterator_tester(200);
    }

    void insert_range_end_input_iterator_tester(std::size_t batch) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.end(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.end(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_end_input_iterator_small_batch_test) {
        insert_range_end_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_end_input_iterator_big_batch_test) {
        insert_range_end_input_iterator_tester(200);
    }

    template<typename T>
    void insert_range_input_iterator_end_exception_safety_tester(auto func) {
        deque_type deq;
        std::deque<constructor_stub> standard = set_up_for_back_exception(deq, func, SMALL_LIMIT / 2);
        insert_range_input_iterator_with_exception<T>(deq, deq.size());
        test_deq_std_deq_operability(deq, standard);
    }

    TEST_F(deque_test, insert_range_input_iterator_end_exception_safety_new_chunk_test) {
        insert_range_input_iterator_end_exception_safety_tester<constructor_stub>([](deque_type& deq) { return deq.__back_capacity(); });
    }

    TEST_F(deque_test, insert_range_input_iterator_end_exception_safety_new_map_test) {
        insert_range_input_iterator_end_exception_safety_tester<constructor_stub*>([](deque_type& deq) { return deq.__back_ghost_capacity(); });
    }

    void insert_range_right_input_iterator_tester(std::size_t batch) {
        deque_type deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_right_input_iterator_small_batch_test) {
        insert_range_right_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_right_input_iterator_big_batch_test) {
        insert_range_right_input_iterator_tester(200);
    }

    TEST_F(deque_test, erase_single_begin_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.erase(deq.begin());
            standard.erase(standard.begin());
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_single_end_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.erase(deq.end() - 1);
            standard.erase(standard.end() - 1);
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_single_left_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0, j = 1; i < MEDIUM_LIMIT / 2; ++i) {
            deq.erase(deq.begin() + j);
            standard.erase(standard.begin() + j);
            j++;
            if ((std::size_t) j >= standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_single_right_test) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0, j = standard.size() + 1; i < MEDIUM_LIMIT / 2; ++i) {
            deq.erase(deq.begin() + j);
            standard.erase(standard.begin() + j);
            j++;
            if ((std::size_t) j >= standard.size()) {
                j = standard.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }
    
    void erase_range_left_test(std::size_t batch) {
        deque_type deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0, j = 1; i + batch + 1 < MEDIUM_LIMIT; i += batch) {
            deq.erase(deq.begin() + j, deq.begin() + j + batch);
            standard.erase(standard.begin() + j, standard.begin() + j + batch);
            j++;
            if ((std::size_t) j >= standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_range_left_small_batch_test) {
        erase_range_left_test(5);
    }

    TEST_F(deque_test, erase_range_right_small_batch_test) {
        erase_range_left_test(100);
    }

    TEST_F(deque_test, begin_end_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        auto it = deq.begin();
        for (int i = 0; it != deq.end(); ++i, ++it) {
            EXPECT_EQ(i, *it);
        }
    }

    TEST_F(deque_test, cbegin_cend_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        auto const_it = deq.cbegin();
        for (int i = 0; const_it != deq.cend(); ++i, ++const_it) {
            EXPECT_EQ(i, *const_it);
        }
    }

    TEST_F(deque_test, foreach_loop_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        for (int& num : deq) {
            num += 1;
        }
        int i = 1;
        for (int& num : deq) {
            EXPECT_EQ(num, i);
            ++i;
        }
    }

    TEST_F(deque_test, const_foreach_loop_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        const deque<int> immutable_deq(deq);
        int i = 0;
        for (const int& num : deq) {
            EXPECT_EQ(num, i);
            ++i;
        }
    }

    deque_type to_stub_deque(vector<int> vec) {
        deque_type stubs;
        for (int num : vec) {
            stubs.emplace_back(num);
        }
        return stubs;
    }

    TEST_F(deque_test, equality_test) {
        deque_type deq1 = to_stub_deque({0, 1, 2});
        deque_type deq2 = to_stub_deque({0, 1, 2});
        EXPECT_EQ(deq1, deq2);
    }

    TEST_F(deque_test, inequality_test) {
        deque_type deq1 = to_stub_deque({0, 1, 2});
        deque_type deq2 = to_stub_deque({0, 1, 3});
        EXPECT_NE(deq1, deq2);
    }

    TEST_F(deque_test, three_way_comparison_test) {
        deque_type deq1 = to_stub_deque({0, 1, 2});
        deque_type deq2 = to_stub_deque({0, 1, 3});
        EXPECT_LT(deq1, deq2);
        EXPECT_LE(deq1, deq2);
        EXPECT_GT(deq2, deq1);
        EXPECT_GE(deq2, deq1);
    }

    TEST_F(deque_test, three_way_comparison_length_test) {
        deque_type deq1 = to_stub_deque({0, 1});
        deque_type deq2 = to_stub_deque({0, 1, 3});
        EXPECT_LT(deq1, deq2);
        EXPECT_LE(deq1, deq2);
        EXPECT_GT(deq2, deq1);
        EXPECT_GE(deq2, deq1);
    }
}
