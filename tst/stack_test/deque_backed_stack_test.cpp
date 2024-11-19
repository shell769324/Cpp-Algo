#include "gtest/gtest.h"
#include "src/deque/deque.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/stack_test/stack_test.h"
#include <iostream>

namespace {
    
    using namespace algo;
    class deque_backed_stack_test : public ::testing::Test {
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    INSTANTIATE_TYPED_TEST_SUITE_P(deque_backed_stack, stack_test, deque<int>);
}
