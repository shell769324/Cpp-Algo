#include "gtest/gtest.h"
#include "tree/red_black_tree_map.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/map_test.h"
#include <iostream>

namespace {
    
    using namespace algo;
    class red_black_tree_map_test : public ::testing::Test {
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

    using stub_red_black_map_type = red_black_tree_map<constructor_stub, constructor_stub, constructor_stub_comparator, tracking_allocator<std::pair<const constructor_stub, constructor_stub> > >;

    INSTANTIATE_TYPED_TEST_SUITE_P(red_black_tree, map_test, stub_red_black_map_type);
}
