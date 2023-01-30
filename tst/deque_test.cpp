#include "gtest/gtest.h"
#include "deque/deque.h"
#include <deque>
#include "tst/utility/constructor_stub.h"

namespace {
    using namespace algo;
    class deque_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
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

    template<typename T>
    void test_deq_std_deq_equality(deque<T>& deq, std::deque<T>& std_deq) {
        EXPECT_EQ(deq.size(), std_deq.size());
        for (std::size_t i = 0; i < deq.size(); i++) {
            EXPECT_EQ(deq[i], std_deq[i]);
        }
    }

    template<typename T>
    void test_deq_std_deq_equality_iterator(deque<T>& deq, std::deque<T>& std_deq) {
        EXPECT_EQ(deq.size(), std_deq.size());
        auto standard_it = std_deq.begin();
        for (auto it = deq.begin(); it != deq.end(); it++, standard_it++) {
            EXPECT_EQ(*standard_it, *it);
        }
    }

    TEST_F(deque_test, default_constructor_test) {
        deque<constructor_stub> deq;
        EXPECT_TRUE(deq.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(deque_test, copy_constructor_test) {
        deque<constructor_stub> deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        deque<constructor_stub> deq_copy(deq);
        test_deq_std_deq_equality(deq, std_deq);
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, copy_constructor_operation_test) {
        deque<constructor_stub> deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        deque<constructor_stub> deq_copy(deq);
        for (int i = 0; i < MEDIUM_LIMIT; i++) {
            deq_copy.emplace_back(i);
            std_deq.emplace_back(i);
        }
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, front_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(constructor_stub(i));
        }
        EXPECT_EQ(deq.front().id, 0);
    }

    TEST_F(deque_test, back_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(constructor_stub(i));
        }
        EXPECT_EQ(deq.back().id, SMALL_LIMIT - 1);
    }

    TEST_F(deque_test, front_alias_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(constructor_stub(i));
        }
        deq.front().id = SPECIAL_VALUE;
        EXPECT_EQ(deq.front().id, SPECIAL_VALUE);
    }

    TEST_F(deque_test, back_alias_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(constructor_stub(i));
        }
        deq.back().id = SPECIAL_VALUE;
        EXPECT_EQ(deq.back().id, SPECIAL_VALUE);
    }

    void push_pop_back_tester(std::size_t limit) {
        deque<constructor_stub> deq;
        for (std::size_t i = 0; i < limit; i++) {
            deq.push_back(constructor_stub(i));
        }
        for (std::size_t i = 0; i < limit; i++) {
            EXPECT_EQ(deq[i].id, i);
        }
        for (std::size_t i = 0; i < limit; i++) {
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
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; j++) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; j++) {
                deq.pop_back();
                standard.pop_back();
            }
        }
    }

    void push_pop_front_tester(std::size_t limit) {
        deque<constructor_stub> deq;
        for (std::size_t i = 0; i < limit; i++) {
            deq.push_front(constructor_stub(i));
        }
        for (std::size_t i = 0; i < limit; i++) {
            EXPECT_EQ(limit - 1 - deq[i].id, i);
        }
        for (std::size_t i = 0; i < limit; i++) {
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
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; j++) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; j++) {
                deq.pop_front();
                standard.pop_front();
            }
        }
    }

    TEST_F(deque_test, all_push_pop_mixed_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; j++) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            for (std::size_t j = 0; j < i; j++) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; j++) {
                deq.pop_front();
                standard.pop_front();
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; j++) {
                deq.pop_back();
                standard.pop_back();
            }
        }
    }

    TEST_F(deque_test, begin_end_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; j++) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            for (std::size_t j = 0; j < i; j++) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality_iterator(deq, standard);
            for (std::size_t j = 0; j < i; j++) {
                deq.pop_front();
                standard.pop_front();
            }
            test_deq_std_deq_equality_iterator(deq, standard);
            for (std::size_t j = 0; j < i; j++) {
                deq.pop_back();
                standard.pop_back();
            }
        }
    }

    TEST_F(deque_test, cbegin_cend_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(i);
        }
        auto const_it = deq.cbegin();
        for (int i = 0; const_it != deq.cend(); i++, const_it++) {
            EXPECT_EQ(i, *const_it);
        }
    }

    TEST_F(deque_test, foreach_loop_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(i);
        }
        for (int& num : deq) {
            num += 1;
        }
        int i = 1;
        for (int& num : deq) {
            EXPECT_EQ(num, i);
            i++;
        }
    }

    TEST_F(deque_test, const_foreach_loop_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            deq.push_back(i);
        }
        deque<int> immutable_deq(deq);
        int i = 0;
        for (const int& num : deq) {
            EXPECT_EQ(num, i);
            i++;
        }
    }
}
