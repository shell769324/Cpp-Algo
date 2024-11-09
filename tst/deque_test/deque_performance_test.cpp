#include "gtest/gtest.h"
#include "deque/deque.h"
#include <deque>
#include "tst/utility/constructor_stub.h"

namespace {
    using namespace algo;
    class deque_performance_test : public ::testing::Test {
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
    static const int LIMIT = 800000;
    static const int MEDIUM_LIMIT = 2000;

    TEST_F(deque_performance_test, fill_constructor_test) {
        deque<constructor_stub> deq(LIMIT, constructor_stub(0));
    }

    TEST_F(deque_performance_test, fill_constructor_standard_test) {
        std::deque<constructor_stub> deq(LIMIT, constructor_stub(0));
    }

    TEST_F(deque_performance_test, push_back_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
    }

    TEST_F(deque_performance_test, push_back_standard_test) {
        std::deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
    }

    TEST_F(deque_performance_test, push_front_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
        }
    }

    TEST_F(deque_performance_test, push_front_standard_test) {
        std::deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
        }
    }
    
    TEST_F(deque_performance_test, shift_left_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
            deq.pop_back();
        }
    }

    TEST_F(deque_performance_test, shift_left_standard_test) {
        std::deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
            deq.pop_back();
        }
    }

    TEST_F(deque_performance_test, shift_right_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
            deq.pop_front();
        }
    }

    TEST_F(deque_performance_test, shift_right_standard_test) {
        std::deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
            deq.pop_front();
        }
    }

    
}
