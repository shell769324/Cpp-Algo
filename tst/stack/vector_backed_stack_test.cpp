#include "gtest/gtest.h"
#include "src/vector/vector.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/stack/stack_test.h"

namespace {
    
    using namespace algo;
    class vector_backed_stack_test : public ::testing::Test {
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

    INSTANTIATE_TYPED_TEST_SUITE_P(vector_backed_stack, stack_test, vector<int>);
}
