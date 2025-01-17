#include "gtest/gtest.h"
#include "src/tree/red_black_tree_set.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/copy_only_constructor_stub.h"
#include "tst/utility/move_only_constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/set_test.h"

namespace {
    
    using namespace algo;
    class red_black_tree_set_test : public ::testing::Test {
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

    struct type_holder {
        using regular_type = red_black_tree_set<constructor_stub, constructor_stub_comparator, tracking_allocator<constructor_stub> >;
        using copy_only_type = red_black_tree_set<copy_only_constructor_stub, std::less<copy_only_constructor_stub>, tracking_allocator<copy_only_constructor_stub> >;
        using move_only_type = red_black_tree_set<move_only_constructor_stub, std::less<move_only_constructor_stub>, tracking_allocator<move_only_constructor_stub> >;
    };

    INSTANTIATE_TYPED_TEST_SUITE_P(red_black_tree, set_test, type_holder);
}
