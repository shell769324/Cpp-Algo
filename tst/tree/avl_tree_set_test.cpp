#include "gtest/gtest.h"
#include "tree/avl_tree_set.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include <iostream>
#include "tst/set_test.h"

namespace {
    
    using namespace algo;
    class avl_tree_set_test : public ::testing::Test {
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

    using stub_avl_set_type = avl_tree_set<constructor_stub, constructor_stub_comparator>;

    INSTANTIATE_TYPED_TEST_SUITE_P(avl_tree_set_interface, set_test, stub_avl_set_type);
}
