#include "gtest/gtest.h"
#include "tree/avl_tree_map.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/copy_only_constructor_stub.h"
#include "tst/utility/move_only_constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/map_test.h"
#include "tst/utility/tracking_allocator.h"
#include <iostream>

namespace {
    
    using namespace algo;
    class avl_tree_map_test : public ::testing::Test {
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }
    };

    struct type_holder {
        using regular_type = avl_tree_map<constructor_stub, constructor_stub, constructor_stub_comparator, tracking_allocator<std::pair<const constructor_stub, constructor_stub> > >;
        using copy_only_type = avl_tree_map<copy_only_constructor_stub, copy_only_constructor_stub, std::less<copy_only_constructor_stub>, tracking_allocator<std::pair<const copy_only_constructor_stub, copy_only_constructor_stub> > >;
        using move_only_type = avl_tree_map<constructor_stub, move_only_constructor_stub, std::less<constructor_stub>, tracking_allocator<std::pair<const constructor_stub, move_only_constructor_stub> > >;
    };

    INSTANTIATE_TYPED_TEST_SUITE_P(avl_tree, map_test, type_holder);
}
