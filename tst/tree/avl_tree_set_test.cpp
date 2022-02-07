#include "gtest/gtest.h"
#include "tree/avl_tree_set.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include <iostream>

namespace {
    
    using namespace algo;
    class avl_tree_set_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    TEST_F(avl_tree_set_test, basic_test) {
        
    }
}

namespace algo {
    template class avl_tree_set<int>;

    template class avl_tree_set<constructor_stub, constructor_stub_comparator>;
}