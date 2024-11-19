#include "gtest/gtest.h"
#include "deque/deque.h"
#include <deque>
#include "tst/utility/constructor_stub.h"

namespace {
    using namespace algo;
    class deque_iterator_test : public ::testing::Test {
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
    template<typename T>
    void test_deq_std_deq_equality(deque<T>& deq, std::deque<T>& std_deq) {
        EXPECT_EQ(deq.size(), std_deq.size());
        for (std::size_t i = 0; i < deq.size(); ++i) {
            EXPECT_EQ(deq[i], std_deq[i]);
        }
    }

    template<typename T>
    void test_deq_std_deq_equality_iterator(deque<T>& deq, std::deque<T>& std_deq) {
        EXPECT_EQ(deq.size(), std_deq.size());
        auto standard_it = std_deq.begin();
        for (auto it = deq.begin(); it != deq.end(); ++it, ++standard_it) {
            EXPECT_EQ(*standard_it, *it);
        }
    }

    constructor_stub** prepare(std::unique_ptr<constructor_stub*[]>& cleaner, std::unique_ptr<std::unique_ptr<constructor_stub[]>[]>& inner_cleaner) {
        cleaner = std::make_unique<constructor_stub*[]>(SMALL_LIMIT);
        inner_cleaner = std::make_unique<std::unique_ptr<constructor_stub[]>[]>(SMALL_LIMIT);
        constructor_stub** data = cleaner.get();
        std::fill(data, data + SMALL_LIMIT, nullptr);
        for (constructor_stub** p = data + 1, **end = data + SMALL_LIMIT - 1; p != end; ++p) {
            *p = new constructor_stub[CHUNK_SIZE<constructor_stub>];
            inner_cleaner.get()[p - data].reset(*p);
        }
        return data;
    }

    TEST_F(deque_iterator_test, trait_test) {
        bool correct = std::is_same_v<deque_iterator<constructor_stub>::iterator_category, std::random_access_iterator_tag>
            && std::is_same_v<deque_iterator<constructor_stub>::value_type, constructor_stub>
            && std::is_same_v<deque_iterator<constructor_stub>::reference, constructor_stub&>
            && std::is_same_v<deque_iterator<constructor_stub>::pointer, constructor_stub*>
            && std::is_same_v<deque_iterator<constructor_stub>::difference_type, std::ptrdiff_t>;
        EXPECT_TRUE(correct);
    }

    TEST_F(deque_iterator_test, default_constructor_test) {
        deque_iterator<int> it1, it2;
        EXPECT_EQ(it1, it2);
    }

    TEST_F(deque_iterator_test, pointer_constructor_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it1(data, data[0]);
        deque_iterator<constructor_stub> it2(data + 1, data[1]);
    }

    TEST_F(deque_iterator_test, reference_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        *it = SPECIAL_VALUE;
        EXPECT_EQ(data[1][0].id, SPECIAL_VALUE);
        deque_iterator<constructor_stub> it2(data + 2, data[2] + 3);
        *it2 = SPECIAL_VALUE;
        EXPECT_EQ(data[2][3].id, SPECIAL_VALUE);
    }

    TEST_F(deque_iterator_test, pointer_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        *it = SPECIAL_VALUE;
        EXPECT_EQ(it -> id, SPECIAL_VALUE);
    }

    TEST_F(deque_iterator_test, pre_increment_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        for (int i = 1, c = 0; i < SMALL_LIMIT - 1; ++i) {
            for (int j = 0; j < CHUNK_SIZE<constructor_stub>; ++j, ++c, ++it) {
                data[i][j] = c;
                EXPECT_EQ(data[i][j].id, it -> id);
            }
        }
    }

    TEST_F(deque_iterator_test, post_increment_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        it -> id = SPECIAL_VALUE;
        auto dup = it++;
        EXPECT_EQ(*dup, SPECIAL_VALUE);
        dup++;
        EXPECT_EQ(dup, it);
    }

    TEST_F(deque_iterator_test, pre_decrement_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + SMALL_LIMIT - 2, data[SMALL_LIMIT - 2] + CHUNK_SIZE<constructor_stub> - 1);
        for (int i = SMALL_LIMIT - 2, c = 0; i >= 1; --i) {
            for (int j = CHUNK_SIZE<constructor_stub> - 1; j >= 0; --j, ++c, --it) {
                data[i][j] = c;
                EXPECT_EQ(data[i][j].id, it -> id);
            }
        }
    }
    
    TEST_F(deque_iterator_test, post_decrement_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        it -> id = SPECIAL_VALUE;
        auto dup = it--;
        EXPECT_EQ(*dup, SPECIAL_VALUE);
        dup--;
        EXPECT_EQ(dup, it);
    }

    TEST_F(deque_iterator_test, plus_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        for (int i = 1, c = 0; i < SMALL_LIMIT - 1; ++i) {
            for (int j = 0; j < CHUNK_SIZE<constructor_stub>; ++j, ++c) {
                data[i][j] = c;
            }
        }
        int cap = CHUNK_SIZE<constructor_stub> * (SMALL_LIMIT - 2);
        for (int i = 0; i < cap; i += PRIME) {
            int row = i / CHUNK_SIZE<constructor_stub> + 1;
            int col = i % CHUNK_SIZE<constructor_stub>;
            EXPECT_EQ(data[row][col].id, (it + i) -> id);
        }
    }

    TEST_F(deque_iterator_test, minus_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + SMALL_LIMIT - 2, data[SMALL_LIMIT - 2] + CHUNK_SIZE<constructor_stub> - 1);
        for (int i = 1, c = 0; i < SMALL_LIMIT - 1; ++i) {
            for (int j = 0; j < CHUNK_SIZE<constructor_stub>; ++j, ++c) {
                data[i][j] = c;
            }
        }
        int cap = CHUNK_SIZE<constructor_stub> * (SMALL_LIMIT - 2);
        for (int i = cap - 1; i >= 0; i -= PRIME) {
            int row = i / CHUNK_SIZE<constructor_stub> + 1;
            int col = i % CHUNK_SIZE<constructor_stub>;
            EXPECT_EQ(data[row][col].id, (it - (cap - 1 - i)) -> id);
        }
    }

    TEST_F(deque_iterator_test, plus_equals_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + 1, data[1]);
        for (int i = 1, c = 0; i < SMALL_LIMIT - 1; ++i) {
            for (int j = 0; j < CHUNK_SIZE<constructor_stub>; ++j, ++c) {
                data[i][j] = c;
            }
        }
        int cap = CHUNK_SIZE<constructor_stub> * (SMALL_LIMIT - 2);
        for (int i = 0; i < cap; i += PRIME, it += PRIME) {
            int row = i / CHUNK_SIZE<constructor_stub> + 1;
            int col = i % CHUNK_SIZE<constructor_stub>;
            EXPECT_EQ(data[row][col].id, it -> id);
        }
    }

    TEST_F(deque_iterator_test, minus_equals_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it(data + SMALL_LIMIT - 2, data[SMALL_LIMIT - 2] + CHUNK_SIZE<constructor_stub> - 1);
        for (int i = 1, c = 0; i < SMALL_LIMIT - 1; ++i) {
            for (int j = 0; j < CHUNK_SIZE<constructor_stub>; ++j, ++c) {
                data[i][j] = c;
            }
        }
        int cap = CHUNK_SIZE<constructor_stub> * (SMALL_LIMIT - 2);
        for (int i = cap - 1; i >= 0; i -= PRIME, it -= PRIME) {
            int row = i / CHUNK_SIZE<constructor_stub> + 1;
            int col = i % CHUNK_SIZE<constructor_stub>;
            EXPECT_EQ(data[row][col].id, it -> id);
        }
    }

    TEST_F(deque_iterator_test, comparison_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it1(data + 1, data[1] + 15);
        deque_iterator<constructor_stub> it2(data + 2, data[2]);
        EXPECT_LE(it1, it2);
        EXPECT_LT(it1, it2);
        EXPECT_NE(it1, it2);
        EXPECT_GE(it2, it1);
        EXPECT_GT(it2, it1);
        EXPECT_NE(it2, it1);
    }

    TEST_F(deque_iterator_test, distance_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub> it1(data + 1, data[1] + 20);
        deque_iterator<constructor_stub> it2(data + 3, data[3] + 20);
        EXPECT_EQ(CHUNK_SIZE<constructor_stub> * 2, it2 - it1);
        EXPECT_EQ(-CHUNK_SIZE<constructor_stub> * 2, it1 - it2);
    }

    TEST_F(deque_iterator_test, reverse_test) {
        std::unique_ptr<constructor_stub*[]> cleaner;
        std::unique_ptr<std::unique_ptr<constructor_stub[]>[]> inner_cleaner;
        constructor_stub** data = prepare(cleaner, inner_cleaner);
        deque_iterator<constructor_stub, true> it(data + 1, data[1]);
        data[1][0] = SPECIAL_VALUE;
        data[1][1] = SPECIAL_VALUE2;
        data[2][0] = SPECIAL_VALUE2;
        --it;
        EXPECT_EQ(*it, SPECIAL_VALUE2);
        ++it;
        EXPECT_EQ(*it, SPECIAL_VALUE);
        it -= CHUNK_SIZE<constructor_stub>;
        EXPECT_EQ(*it, SPECIAL_VALUE2);
        it += CHUNK_SIZE<constructor_stub>;
        EXPECT_EQ(*it, SPECIAL_VALUE);
        deque_iterator<constructor_stub, true> it2 = it - CHUNK_SIZE<constructor_stub> * 4 / 3;
        EXPECT_LT(it2, it);
        EXPECT_GT(it, it2);
    }
}
