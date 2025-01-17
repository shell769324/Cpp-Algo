#include "gtest/gtest.h"
#include "src/vector/vector_iterator.h"
#include "tst/utility/constructor_stub.h"

namespace {
    using namespace algo;
    class vector_iterator_test : public ::testing::Test {
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
    static const int PRIME = 19;
    static const int MEDIUM_LIMIT = 1000;
    static const int SMALL_LIMIT = 10;
    constructor_stub* prepare(std::unique_ptr<constructor_stub[]>& cleaner, int size=SMALL_LIMIT) {
        cleaner = std::make_unique<constructor_stub[]>(size);
        for (int i = 0; i < size; ++i) {
            cleaner[i] = i;
        }
        return cleaner.get();
    }

    TEST_F(vector_iterator_test, trait_test) {
        bool correct = std::is_same_v<vector_iterator<constructor_stub>::iterator_category, std::contiguous_iterator_tag>
            && std::is_same_v<vector_iterator<constructor_stub>::value_type, constructor_stub>
            && std::is_same_v<vector_iterator<constructor_stub>::reference, constructor_stub&>
            && std::is_same_v<vector_iterator<constructor_stub>::pointer, constructor_stub*>
            && std::is_same_v<vector_iterator<constructor_stub>::difference_type, std::ptrdiff_t>;
        EXPECT_TRUE(correct);
        static_assert(std::contiguous_iterator<vector_iterator<constructor_stub> >);
        static_assert(std::contiguous_iterator<vector_iterator<constructor_stub, true> >);
    }

    TEST_F(vector_iterator_test, default_constructor_test) {
        vector_iterator<int> it1, it2;
        EXPECT_EQ(it1, it2);
    }

    TEST_F(vector_iterator_test, pointer_constructor_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it1(data);
        vector_iterator<constructor_stub> it2(data + 1);
    }

    TEST_F(vector_iterator_test, subscript_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        data[1].id = SPECIAL_VALUE;
        vector_iterator<constructor_stub> it(data);
        it[1].id = -SPECIAL_VALUE;
        EXPECT_EQ(it[1].id, -SPECIAL_VALUE);
    }

    TEST_F(vector_iterator_test, const_subscript_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        data[1].id = SPECIAL_VALUE;
        vector_iterator<constructor_stub> it(data);
        EXPECT_EQ(it[1].id, SPECIAL_VALUE);
        vector_iterator<constructor_stub> it2(data + 2);
        EXPECT_EQ(it2[-1].id, SPECIAL_VALUE);
    }

    TEST_F(vector_iterator_test, reference_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data + 2);
        *it = SPECIAL_VALUE;
        EXPECT_EQ(data[2].id, SPECIAL_VALUE);
    }

    TEST_F(vector_iterator_test, pointer_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data + 2);
        *it = SPECIAL_VALUE;
        EXPECT_EQ(it -> id, SPECIAL_VALUE);
    }

    TEST_F(vector_iterator_test, pre_increment_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data);
        for (int i = 0, c = 1; i < SMALL_LIMIT; ++i, ++it) {
            data[i] = c;
            EXPECT_EQ(data[i].id, it -> id);
        }
    }

    TEST_F(vector_iterator_test, post_increment_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data + 1);
        it -> id = SPECIAL_VALUE;
        auto dup = it++;
        EXPECT_EQ(*dup, SPECIAL_VALUE);
        dup++;
        EXPECT_EQ(dup, it);
    }

    TEST_F(vector_iterator_test, pre_decrement_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data + SMALL_LIMIT - 1);
        for (int i = SMALL_LIMIT - 1; i >= 0; --i, --it) {
            EXPECT_EQ(data[i].id, it -> id);
        }
    }
    
    TEST_F(vector_iterator_test, post_decrement_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data + 1);
        it -> id = SPECIAL_VALUE;
        auto dup = it--;
        EXPECT_EQ(*dup, SPECIAL_VALUE);
        dup--;
        EXPECT_EQ(dup, it);
    }

    TEST_F(vector_iterator_test, plus_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            EXPECT_EQ(data[i].id, (it + i) -> id);
        }
        it += SMALL_LIMIT - 1;
        for (int i = 0; i <= SMALL_LIMIT - 1; ++i) {
            EXPECT_EQ(data[SMALL_LIMIT - 1 - i].id, (it + -i) -> id);
        }
    }

    TEST_F(vector_iterator_test, minus_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            EXPECT_EQ(data[i].id, (it - -i) -> id);
        }
        it += SMALL_LIMIT - 1;
        for (int i = 0; i <= SMALL_LIMIT - 1; ++i) {
            EXPECT_EQ(data[SMALL_LIMIT - 1 - i].id, (it - i) -> id);
        }
    }

    TEST_F(vector_iterator_test, plus_equals_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            auto copy = it;
            copy += i;
            EXPECT_EQ(data[i].id, copy -> id);
        }
        it += SMALL_LIMIT - 1;
        for (int i = 0; i <= SMALL_LIMIT - 1; ++i) {
            auto copy = it;
            copy += -i;
            EXPECT_EQ(data[SMALL_LIMIT - 1 - i].id, copy -> id);
        }
    }

    TEST_F(vector_iterator_test, minus_equals_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it(data);
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            auto copy = it;
            copy -= -i;
            EXPECT_EQ(data[i].id, copy -> id);
        }
        it += SMALL_LIMIT - 1;
        for (int i = 0; i <= SMALL_LIMIT - 1; ++i) {
            auto copy = it;
            copy -= i;
            EXPECT_EQ(data[SMALL_LIMIT - 1 - i].id, copy -> id);
        }
    }

    TEST_F(vector_iterator_test, comparison_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it1(data + 1);
        vector_iterator<constructor_stub> it2(data + 2);
        EXPECT_LE(it1, it2);
        EXPECT_LT(it1, it2);
        EXPECT_NE(it1, it2);
        EXPECT_GE(it2, it1);
        EXPECT_GT(it2, it1);
        EXPECT_NE(it2, it1);
    }

    TEST_F(vector_iterator_test, distance_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub> it1(data + 1);
        vector_iterator<constructor_stub> it2(data + 5);
        EXPECT_EQ(4, it2 - it1);
        EXPECT_EQ(-4, it1 - it2);
    }

    TEST_F(vector_iterator_test, reverse_test) {
        std::unique_ptr<constructor_stub[]> cleaner;
        
        constructor_stub* data = prepare(cleaner);
        vector_iterator<constructor_stub, true> it(data + 2);
        ++it;
        EXPECT_EQ(*it, 1);
        --it;
        EXPECT_EQ(*it, 2);
        it += 2;
        EXPECT_EQ(*it, 0);
        it -= 2;
        EXPECT_EQ(*it, 2);
        vector_iterator<constructor_stub, true> it2 = it - 1;
        EXPECT_LT(it2, it);
        EXPECT_GT(it, it2);
        EXPECT_EQ(it - it2, 1);
    }
}
