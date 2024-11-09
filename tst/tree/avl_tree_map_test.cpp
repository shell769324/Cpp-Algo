#include "gtest/gtest.h"
#include "tree/avl_tree_map.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/map_test.h"
#include <iostream>

namespace {
    
    using namespace algo;
    class avl_tree_map_test : public ::testing::Test {
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

    using stub_avl_map_type = avl_tree_map<constructor_stub, constructor_stub, constructor_stub_comparator>;

    INSTANTIATE_TYPED_TEST_SUITE_P(avl_tree_map_interface, map_test, stub_avl_map_type);
}
