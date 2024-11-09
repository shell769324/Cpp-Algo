#include "gtest/gtest.h"
#include "deque/deque.h"
#include <deque>
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"

namespace {
    using namespace algo;
    class deque_test : public ::testing::Test {
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
    static const int BIG_PRIME = 7759;
    static const int MEDIUM_PRIME = 443;
    static const int SMALL_PRIME = 19;
    static const int LIMIT = 10000;
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

    TEST_F(deque_test, default_constructor_test) {
        deque<constructor_stub> deq;
        EXPECT_TRUE(deq.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(deque_test, copy_constructor_test) {
        deque<constructor_stub> deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        deque<constructor_stub> deq_copy(deq);
        test_deq_std_deq_equality(deq, std_deq);
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, copy_constructor_operation_test) {
        deque<constructor_stub> deq(MEDIUM_LIMIT);
        std::deque<constructor_stub> std_deq(deq.begin(), deq.end());
        deque<constructor_stub> deq_copy(deq);
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq_copy.emplace_back(i);
            std_deq.emplace_back(i);
        }
        test_deq_std_deq_equality(deq_copy, std_deq);
    }

    TEST_F(deque_test, front_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        EXPECT_EQ(deq.front().id, 0);
    }

    TEST_F(deque_test, back_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        EXPECT_EQ(deq.back().id, SMALL_LIMIT - 1);
    }

    TEST_F(deque_test, front_alias_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        deq.front().id = SPECIAL_VALUE;
        EXPECT_EQ(deq.front().id, SPECIAL_VALUE);
    }

    TEST_F(deque_test, back_alias_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
        }
        deq.back().id = SPECIAL_VALUE;
        EXPECT_EQ(deq.back().id, SPECIAL_VALUE);
    }

    TEST_F(deque_test, push_back_move_test) {
        deque<constructor_stub> deq;
        deq.push_back(constructor_stub(0));
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 2);
    }

    TEST_F(deque_test, emplace_back_test) {
        deque<constructor_stub> deq;
        deq.emplace_back(0);
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 1);
    }

    TEST_F(deque_test, push_front_move_test) {
        deque<constructor_stub> deq;
        deq.push_front(constructor_stub(0));
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 2);
    }

    TEST_F(deque_test, emplace_front_test) {
        deque<constructor_stub> deq;
        deq.emplace_front(0);
        EXPECT_EQ(constructor_stub::id_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 1);
    }

    TEST_F(deque_test, shift_left_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
            deq.pop_back();
            EXPECT_EQ(deq.__get_num_active_chunks(), 0);
            EXPECT_LE(deq.__get_num_chunks(), 3);
        }
    }

    TEST_F(deque_test, shift_left_gap_test) {
        deque<constructor_stub> deq;
        deq.push_front(constructor_stub(0));
        for (int i = 1; i < LIMIT; ++i) {
            deq.push_front(constructor_stub(i));
            deq.pop_back();
            EXPECT_EQ(deq.__get_num_active_chunks(), 1);
            EXPECT_LE(deq.__get_num_chunks(), 6);
        }
    }

    TEST_F(deque_test, shift_right_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
            deq.pop_front();
            EXPECT_EQ(deq.__get_num_active_chunks(), 0);
            EXPECT_LE(deq.__get_num_chunks(), 3);
        }
    }

    TEST_F(deque_test, shift_right_gap_test) {
        deque<constructor_stub> deq;
        deq.push_back(constructor_stub(0));
        for (int i = 1; i < LIMIT; ++i) {
            deq.push_back(constructor_stub(i));
            deq.pop_front();
            EXPECT_EQ(deq.__get_num_active_chunks(), 1);
            EXPECT_LE(deq.__get_num_chunks(), 6);
        }
    }

    void push_pop_back_tester(std::size_t limit) {
        deque<constructor_stub> deq;
        for (std::size_t i = 0; i < limit; ++i) {
            deq.push_back(constructor_stub(i));
        }
        for (std::size_t i = 0; i < limit; ++i) {
            EXPECT_EQ(deq[i].id, i);
        }
        for (std::size_t i = 0; i < limit; ++i) {
            deq.pop_back();
        }
    }

    TEST_F(deque_test, push_pop_back_test) {
        push_pop_back_tester(SMALL_LIMIT);
    }

    TEST_F(deque_test, push_pop_back_intermediate_test) {
        push_pop_back_tester(MEDIUM_LIMIT);
    }

    TEST_F(deque_test, push_pop_back_stress_test) {
        push_pop_back_tester(LIMIT);
    }

    TEST_F(deque_test, push_pop_back_mixed_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_back();
                standard.pop_back();
            }
        }
    }

    void push_pop_front_tester(std::size_t limit) {
        deque<constructor_stub> deq;
        for (std::size_t i = 0; i < limit; ++i) {
            deq.push_front(constructor_stub(i));
        }
        for (std::size_t i = 0; i < limit; ++i) {
            EXPECT_EQ(limit - 1 - deq[i].id, i);
        }
        for (std::size_t i = 0; i < limit; ++i) {
            deq.pop_front();
        }
    }

    TEST_F(deque_test, push_pop_front_test) {
        push_pop_front_tester(SMALL_LIMIT);
    }
    
    TEST_F(deque_test, push_pop_front_intermediate_test) {
        push_pop_front_tester(MEDIUM_LIMIT);
    }

    TEST_F(deque_test, push_pop_front_stress_test) {
        push_pop_front_tester(LIMIT);
    }

    TEST_F(deque_test, push_pop_front_mixed_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_front();
                standard.pop_front();
            }
        }
    }

    TEST_F(deque_test, all_push_pop_mixed_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (std::size_t i = 1; i < LIMIT; i *= 2) {
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_front(constructor_stub(j));
                standard.push_front(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_back();
                standard.pop_back();
            }
            EXPECT_EQ(0, deq.__get_num_active_chunks());
            for (std::size_t j = 0; j < i; ++j) {
                deq.push_back(constructor_stub(j));
                standard.push_back(constructor_stub(j));
            }
            test_deq_std_deq_equality(deq, standard);
            for (std::size_t j = 0; j < i; ++j) {
                deq.pop_front();
                standard.pop_front();
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_single_move_test) {
        deque<constructor_stub> deq;
        deq.insert(deq.begin(), constructor_stub(0));
        EXPECT_EQ(1, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(2, constructor_stub::constructor_invocation_count);
    }

    TEST_F(deque_test, insert_emplace_test) {
        deque<constructor_stub> deq;
        deq.emplace(deq.begin(), 0);
        EXPECT_EQ(1, constructor_stub::id_constructor_invocation_count);
        EXPECT_EQ(1, constructor_stub::constructor_invocation_count);
    }

    TEST_F(deque_test, insert_single_begin_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.begin(), constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            EXPECT_EQ(MEDIUM_LIMIT - deq[i].id - 1, i);
        }
    }

    TEST_F(deque_test, insert_single_end_test) {
        deque<constructor_stub> deq;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.end(), constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            EXPECT_EQ(deq[i].id, i);
        }
    }

    TEST_F(deque_test, insert_single_left_test) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.begin() + j, constructor_stub(i));
            standard.insert(standard.begin() + j, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_single_right_test) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; ++i) {
            deq.insert(deq.begin() + j, constructor_stub(i));
            standard.insert(standard.begin() + j, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    void insert_fill_begin_tester(std::size_t batch) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin(), batch, constructor_stub(i));
            standard.insert(standard.begin(), batch, constructor_stub(i));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_begin_small_batch_test) {
        insert_fill_begin_tester(5);
    }

    TEST_F(deque_test, insert_fill_begin_big_batch_test) {
        insert_fill_begin_tester(200);
    }

    void insert_fill_end_tester(std::size_t batch) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.end(), batch, constructor_stub(i));
            standard.insert(standard.end(), batch, constructor_stub(i));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_end_small_batch_test) {
        insert_fill_end_tester(5);
    }

    TEST_F(deque_test, insert_fill_end_big_batch_test) {
        insert_fill_end_tester(200);
    }

    void insert_fill_left_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, batch, constructor_stub(i));
            standard.insert(standard.begin() + j, batch, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_left_small_batch_test) {
        insert_fill_left_tester(5);
    }

    TEST_F(deque_test, insert_fill_left_big_batch_test) {
        insert_fill_left_tester(200);
    }

    void insert_fill_right_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, batch, constructor_stub(i));
            standard.insert(standard.begin() + j, batch, constructor_stub(i));
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_fill_right_small_batch_test) {
        insert_fill_right_tester(5);
    }

    TEST_F(deque_test, insert_fill_right_big_batch_test) {
        insert_fill_right_tester(200);
    }

    void insert_range_begin_tester(std::size_t batch) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin(), content.begin(), content.end());
            standard.insert(standard.begin(), content.begin(), content.end());
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_begin_small_batch_test) {
        insert_range_begin_tester(5);
    }

    TEST_F(deque_test, insert_range_begin_big_batch_test) {
        insert_range_begin_tester(200);
    }

    void insert_range_end_tester(std::size_t batch) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.end(), content.begin(), content.end());
            standard.insert(standard.end(), content.begin(), content.end());
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_end_small_batch_test) {
        insert_range_end_tester(5);
    }

    TEST_F(deque_test, insert_range_end_big_batch_test) {
        insert_range_end_tester(200);
    }

    void insert_range_left_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, content.begin(), content.end());
            standard.insert(standard.begin() + j, content.begin(), content.end());
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_left_small_batch_test) {
        insert_range_left_tester(5);
    }

    TEST_F(deque_test, insert_range_left_big_batch_test) {
        insert_range_left_tester(200);
    }

    void insert_range_right_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> content(batch);
        for (std::size_t i = 0; i < batch; i++) {
            content.emplace_back(i);
        }
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, content.begin(), content.end());
            standard.insert(standard.begin() + j, content.begin(), content.end());
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_right_small_batch_test) {
        insert_range_right_tester(5);
    }

    TEST_F(deque_test, insert_range_right_big_batch_test) {
        insert_range_right_tester(200);
    }

    void insert_range_begin_input_iterator_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.begin(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_begin_input_iterator_small_batch_test) {
        insert_range_begin_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_begin_input_iterator_big_batch_test) {
        insert_range_begin_input_iterator_tester(200);
    }

    void insert_range_left_input_iterator_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            j++;
            if ((std::size_t) j == standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_left_input_iterator_small_batch_test) {
        insert_range_left_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_left_input_iterator_big_batch_test) {
        insert_range_left_input_iterator_tester(200);
    }

    void insert_range_end_input_iterator_tester(std::size_t batch) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.end(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.end(), stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_end_input_iterator_small_batch_test) {
        insert_range_end_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_end_input_iterator_big_batch_test) {
        insert_range_end_input_iterator_tester(200);
    }

    void insert_range_right_input_iterator_tester(std::size_t batch) {
        deque<constructor_stub> deq(SMALL_LIMIT, constructor_stub(0));
        std::deque<constructor_stub> standard(SMALL_LIMIT, constructor_stub(0));
        for (int i = 0, j = deq.size() / 2 + 1; i < MEDIUM_LIMIT; i += batch) {
            deq.insert(deq.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            standard.insert(standard.begin() + j, stub_iterator<constructor_stub>(0), stub_iterator<constructor_stub>(batch));
            j++;
            if ((std::size_t) j == standard.size()) {
                j = deq.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, insert_range_right_input_iterator_small_batch_test) {
        insert_range_right_input_iterator_tester(5);
    }

    TEST_F(deque_test, insert_range_right_input_iterator_big_batch_test) {
        insert_range_right_input_iterator_tester(200);
    }

    TEST_F(deque_test, erase_single_begin_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.erase(deq.begin());
            standard.erase(standard.begin());
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_single_end_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.erase(deq.end() - 1);
            standard.erase(standard.end() - 1);
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_single_left_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0, j = 1; i < MEDIUM_LIMIT / 2; ++i) {
            deq.erase(deq.begin() + j);
            standard.erase(standard.begin() + j);
            j++;
            if ((std::size_t) j >= standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_single_right_test) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0, j = standard.size() + 1; i < MEDIUM_LIMIT / 2; ++i) {
            deq.erase(deq.begin() + j);
            standard.erase(standard.begin() + j);
            j++;
            if ((std::size_t) j >= standard.size()) {
                j = standard.size() / 2 + 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }
    
    void erase_range_left_test(std::size_t batch) {
        deque<constructor_stub> deq;
        std::deque<constructor_stub> standard;
        for (int i = 0; i < MEDIUM_LIMIT; ++i) {
            deq.emplace_back(constructor_stub(i));
            standard.emplace_back(constructor_stub(i));
        }
        for (int i = 0, j = 1; i + batch + 1 < MEDIUM_LIMIT; i += batch) {
            deq.erase(deq.begin() + j, deq.begin() + j + batch);
            standard.erase(standard.begin() + j, standard.begin() + j + batch);
            j++;
            if ((std::size_t) j >= standard.size() / 2) {
                j = 1;
            }
            test_deq_std_deq_equality(deq, standard);
        }
    }

    TEST_F(deque_test, erase_range_left_small_batch_test) {
        erase_range_left_test(5);
    }

    TEST_F(deque_test, erase_range_right_small_batch_test) {
        erase_range_left_test(100);
    }

    TEST_F(deque_test, begin_end_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        auto it = deq.begin();
        for (int i = 0; it != deq.end(); ++i, ++it) {
            EXPECT_EQ(i, *it);
        }
    }

    TEST_F(deque_test, cbegin_cend_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        auto const_it = deq.cbegin();
        for (int i = 0; const_it != deq.cend(); ++i, ++const_it) {
            EXPECT_EQ(i, *const_it);
        }
    }

    TEST_F(deque_test, foreach_loop_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        for (int& num : deq) {
            num += 1;
        }
        int i = 1;
        for (int& num : deq) {
            EXPECT_EQ(num, i);
            ++i;
        }
    }

    TEST_F(deque_test, const_foreach_loop_test) {
        deque<int> deq;
        for (int i = 0; i < SMALL_LIMIT; ++i) {
            deq.push_back(i);
        }
        const deque<int> immutable_deq(deq);
        int i = 0;
        for (const int& num : deq) {
            EXPECT_EQ(num, i);
            ++i;
        }
    }
}
