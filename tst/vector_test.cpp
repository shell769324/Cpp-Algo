#include "gtest/gtest.h"
#include "vector.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/forward_stub.h"
#include "tst/utility/stub_iterator.h"
#include <vector>

namespace {
    using namespace algo;
    class vector_test : public ::testing::Test {
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
    static const int MEDIUM_LIMIT = 500;
    static const int SMALL_LIMIT = 10;

    template<typename T>
    void test_vec_std_vec_equality(vector<T>& vec, std::vector<T>& std_vec) {
        EXPECT_EQ(vec.size(), std_vec.size());
        for (std::size_t i = 0; i < vec.size(); i++) {
            EXPECT_EQ(vec[i], std_vec[i]);
        }
    }

    TEST_F(vector_test, default_constructor_test) {
        vector<constructor_stub> vec;
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(vector_test, fill_default_constructor_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(vector_test, fill_copy_constructor_test) {
        constructor_stub stub;
        vector<constructor_stub> vec(SMALL_LIMIT, stub);
        for (auto& elem : vec) {
            EXPECT_EQ(stub, elem);
        }
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
    }

    TEST_F(vector_test, copy_constructor_test) {
        vector<constructor_stub> vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        vector<constructor_stub> vec_copy(vec);
        test_vec_std_vec_equality(vec, std_vec);
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, move_constructor_test) {
        vector<constructor_stub> vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        vector<constructor_stub> vec_copy(std::move(vec));
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_vec_std_vec_equality(vec_copy, std_vec);
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, range_constructor_test) {
        std::vector<constructor_stub> std_vec(MEDIUM_LIMIT);
        vector<constructor_stub> vec(std_vec.begin(), std_vec.end());
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, initializer_constructor_test) {
        std::vector<int> std_vec{1, 2, 3, 4, 5};
        vector<int> vec{1, 2, 3, 4, 5};
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, assignment_operator_test) {
        vector<constructor_stub> vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        vector<constructor_stub> vec_copy;
        vec_copy = vec;
        test_vec_std_vec_equality(vec, std_vec);
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, move_assignment_operator_test) {
        vector<constructor_stub> vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        int constructor_count_before = constructor_stub::constructor_invocation_count;
        vector<constructor_stub> vec_copy;
        vec_copy = std::move(vec);
        int constructor_count_after = constructor_stub::constructor_invocation_count;
        EXPECT_EQ(constructor_count_before, constructor_count_after);
        test_vec_std_vec_equality(vec_copy, std_vec);
    }

    TEST_F(vector_test, push_back_primitive_test) {
        vector<int> vec;
        vec.push_back(1);
        EXPECT_EQ(vec[0], 1);
    }


    TEST_F(vector_test, push_back_lvalue_basic_test) {
        vector<constructor_stub> vec;
        constructor_stub constructor_stub;
        vec.push_back(constructor_stub);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ(vec.size(), 1);
    }

    TEST_F(vector_test, push_back_rvalue_basic_test) {
        vector<constructor_stub> vec;
        vec.push_back(constructor_stub());
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(vec.size(), 1);
    }

    TEST_F(vector_test, push_back_primitive_stress_test) {
        vector<int> vec;
        for (int i = 0; i < LIMIT; i++) {
            vec.push_back(i);
        }
        for (int i = 0; i < LIMIT; i++) {
            EXPECT_EQ(vec[i], i);
        }
    }

    TEST_F(vector_test, push_back_stress_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < LIMIT; i++) {
            vec.push_back(constructor_stub(i));
        }
        for (int i = 0; i < LIMIT; i++) {
            EXPECT_EQ(vec[i].id, i);
        }
    }


    TEST_F(vector_test, pop_back_basic_test) {
        vector<constructor_stub> vec;
        vec.push_back(constructor_stub());
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 1);
        vec.pop_back();
        // Once when the temporary constructor_stub goes out of scope
        // Once in pop_back()
        EXPECT_EQ(constructor_stub::destructor_invocation_count, 2);
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, pop_back_stress_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < LIMIT; i++) {
            vec.push_back(constructor_stub());
        }
        for (int i = 0; i < LIMIT; i++) {
            vec.pop_back();
        }
        
    }

    TEST_F(vector_test, push_pop_stress_test) {
        vector<constructor_stub> vec;
        std::vector<constructor_stub> vec_copy;
        int rep = 20;
        int limit = 1000;
        for (int i = 0; i < rep; i++) {
            for (int j = 0; j < limit; j++) {
                constructor_stub constructor_stub;
                vec.push_back(constructor_stub);
                vec_copy.push_back(constructor_stub);
            }
            std::size_t half_size = vec.size() / 2;
            for (std::size_t j = 0; j < half_size; j++) {
                vec.pop_back();
                vec_copy.pop_back();
            }
            test_vec_std_vec_equality(vec, vec_copy);
        }
        while (!vec.empty()) {
            vec.pop_back();
            vec_copy.pop_back();
        }
        EXPECT_EQ(vec.size(), 0);
        
    }

    TEST_F(vector_test, empty_test) {
        vector<constructor_stub> vec;
        EXPECT_TRUE(vec.empty());
        vec.push_back(constructor_stub());
        EXPECT_FALSE(vec.empty());
        vec.pop_back();
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, size_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < LIMIT; i++) {
            vec.push_back(constructor_stub());
            EXPECT_EQ(vec.size(), i + 1);
        }
        for (int i = 0; i < LIMIT; i++) {
            vec.pop_back();
            EXPECT_EQ(vec.size(), LIMIT - i - 1);
        }
    }

    TEST_F(vector_test, front_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(constructor_stub(i));
        }
        EXPECT_EQ(vec.front().id, 0);
    }

    TEST_F(vector_test, back_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(constructor_stub(i));
        }
        EXPECT_EQ(vec.back().id, SMALL_LIMIT - 1);
    }

    TEST_F(vector_test, front_alias_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(constructor_stub(i));
        }
        vec.front().id = SPECIAL_VALUE;
        EXPECT_EQ(vec.front().id, SPECIAL_VALUE);
    }

    TEST_F(vector_test, back_alias_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(constructor_stub(i));
        }
        vec.back().id = SPECIAL_VALUE;
        EXPECT_EQ(vec.back().id, SPECIAL_VALUE);
    }

    TEST_F(vector_test, begin_end_test) {
        vector<int> vec;
        std::vector<int> src;
        src.resize(BIG_PRIME);
        // Multiples of coprime numbers should complete the congruence class of each
        for (int i = 0, j = 0; i < BIG_PRIME; i++, j = (j + SMALL_PRIME) % BIG_PRIME) {
            src[j] = i;
        }
        while(!src.empty()) {
            vec.push_back(src.back());
            src.pop_back();
        }
        std::sort(vec.begin(), vec.end());
        for (std::size_t i = 0; i < vec.size(); i++) {
            EXPECT_EQ(vec[i], i);
        }
    }

    TEST_F(vector_test, cbegin_cend_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(i);
        }
        auto const_it = vec.cbegin();
        for (int i = 0; const_it != vec.cend(); i++, const_it++) {
            EXPECT_EQ(i, *const_it);
        }
    }

    TEST_F(vector_test, foreach_loop_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(i);
        }
        for (int& num : vec) {
            num += 1;
        }
        int i = 1;
        for (int& num : vec) {
            EXPECT_EQ(num, i);
            i++;
        }
    }

    TEST_F(vector_test, const_foreach_loop_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(i);
        }
        vector<int> immutable_vec(vec);
        int i = 0;
        for (const int& num : vec) {
            EXPECT_EQ(num, i);
            i++;
        }
    }

    TEST_F(vector_test, clear_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(i);
        }
        vec.clear();
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, insert_single_basic_primitive_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(i);
        }
        vec.insert(vec.begin(), SPECIAL_VALUE);
        EXPECT_EQ(vec[0], SPECIAL_VALUE);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            EXPECT_EQ(vec[i + 1], i);
        }
    }

    TEST_F(vector_test, insert_single_return_value_test) {
        vector<int> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(i);
        }
        vector<int>::iterator it = vec.insert(vec.begin(), SPECIAL_VALUE);
        EXPECT_EQ(*it, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin());
    }

    TEST_F(vector_test, insert_single_basic_test) {
        vector<constructor_stub> vec;
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec.push_back(constructor_stub(i));
        }
        vec.insert(vec.begin(), constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(vec[0].id, SPECIAL_VALUE);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            EXPECT_EQ(vec[i + 1].id, i);
        }
    }

    TEST_F(vector_test, insert_single_stress_test) {
        vector<int> vec;
        std::vector<int> std_vec;
        for (int i = 0; i < MEDIUM_PRIME; i++) {
            vec.push_back(i);
            std_vec.push_back(i);
        }
        for (int i = 0, j = 0; i < MEDIUM_PRIME; i++, j = (j + SMALL_PRIME) % MEDIUM_PRIME) {
            vec.insert(vec.begin() + j, SPECIAL_VALUE);
            std_vec.insert(std_vec.begin() + j, SPECIAL_VALUE);
        }
        test_vec_std_vec_equality(vec, std_vec);        
    }

    TEST_F(vector_test, insert_range_forward_iterator_basic_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> src(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t insert_index = SMALL_LIMIT / 2;
        vec.insert(vec.begin() + insert_index, src.begin(), src.end());
        std_vec.insert(std_vec.begin() + insert_index, src.begin(), src.end());
        test_vec_std_vec_equality(vec, std_vec);        
    }

    TEST_F(vector_test, insert_range_forward_iterator_return_value_test) {
        vector<int> vec(SMALL_LIMIT);
        std::vector<int> src(SMALL_LIMIT);
        src[0] = SPECIAL_VALUE;
        size_t insert_index = SMALL_LIMIT / 2;
        vector<int>::iterator it = vec.insert(vec.begin() + insert_index,
            src.begin(), src.end());
        EXPECT_EQ(*it, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin() + insert_index);
    }

    TEST_F(vector_test, insert_range_forward_iterator_stress_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> src{
            constructor_stub(SPECIAL_VALUE), constructor_stub(~SPECIAL_VALUE),
            constructor_stub(SPECIAL_VALUE2), constructor_stub(~SPECIAL_VALUE2)};
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        for (int i = 0, pos = 0; i < MEDIUM_LIMIT; i++, pos = (pos + SMALL_PRIME) % vec.size()) {
            vec.insert(vec.begin() + pos, src.begin(), src.end());
            std_vec.insert(std_vec.begin() + pos, src.begin(), src.end());
        }
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_input_iterator_basic_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t insert_index = SMALL_LIMIT / 2;
        vec.insert(vec.begin() + insert_index, stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
        std_vec.insert(std_vec.begin() + insert_index, stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, insert_range_input_iterator_return_value_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        size_t insert_index = SMALL_LIMIT / 2;
        vector<constructor_stub>::iterator it = vec.insert(vec.begin() + insert_index,
            stub_iterator<constructor_stub>(SPECIAL_VALUE),
            stub_iterator<constructor_stub>(SPECIAL_VALUE + 1));
        EXPECT_EQ(it -> id, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin() + insert_index);
    }

    TEST_F(vector_test, insert_range_input_iterator_stress_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        for (int i = 0, pos = 0; i < MEDIUM_LIMIT; i++, pos = (pos + SMALL_PRIME) % vec.size()) {
            vec.insert(vec.begin() + pos, stub_iterator<constructor_stub>(SPECIAL_VALUE),
                stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
            std_vec.insert(std_vec.begin() + pos, stub_iterator<constructor_stub>(SPECIAL_VALUE),
                stub_iterator<constructor_stub>(SPECIAL_VALUE + SMALL_LIMIT));
        }
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, erase_single_basic_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        int idx = SMALL_LIMIT / 2;
        vec.erase(vec.begin() + idx);
        std_vec.erase(std_vec.begin() + idx);
        test_vec_std_vec_equality(vec, std_vec);        
    }

    TEST_F(vector_test, erase_single_return_value_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        for (int i = 0; i < SMALL_LIMIT; i++) {
            vec[i].id = i;
        }
        int idx = SMALL_LIMIT / 2;
        int expected_id = vec[idx + 1].id;
        vector<constructor_stub>::iterator it = vec.erase(vec.begin() + idx);
        EXPECT_EQ(it -> id, expected_id);
    }

    TEST_F(vector_test, erase_single_stress_test) {
        vector<int> vec;
        std::vector<int> std_vec;
        for (int i = 0; i < MEDIUM_PRIME; i++) {
            vec.push_back(i);
            std_vec.push_back(i);
        }
        int left_size = MEDIUM_LIMIT / 2;
        for (int i = 0, j = 0; i < left_size; i++, j = (j + SMALL_PRIME) % std_vec.size()) {
            vec.erase(vec.begin() + j);
            std_vec.erase(std_vec.begin() + j);
        }
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, erase_range_basic_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        size_t erase_start = SMALL_LIMIT / 4;
        size_t erase_end = SMALL_LIMIT - erase_start;
        vec.erase(vec.begin() + erase_start, vec.begin() + erase_end);
        std_vec.erase(std_vec.begin() + erase_start, std_vec.begin() + erase_end);
        test_vec_std_vec_equality(vec, std_vec);
    }

    TEST_F(vector_test, erase_range_return_value_test) {
        vector<int> vec(SMALL_LIMIT);
        size_t erase_start = SMALL_LIMIT / 4;
        size_t erase_end = SMALL_LIMIT - erase_start;
        vec[erase_end] = SPECIAL_VALUE;
        vector<int>::iterator it =
            vec.erase(vec.begin() + erase_start, vec.begin() + erase_end);
        EXPECT_EQ(*it, SPECIAL_VALUE);
        EXPECT_EQ(it, vec.begin() + erase_start);
    }

    TEST_F(vector_test, erase_range_stress_test) {
        vector<constructor_stub> vec(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        std::size_t idx = 0;
        while (!std_vec.empty()) {
            std::size_t removed_count = std::min<int>(std_vec.size(), SMALL_LIMIT);
            vec.erase(vec.begin() + idx, vec.begin() + idx + removed_count);
            std_vec.erase(std_vec.begin() + idx, std_vec.begin() + idx + removed_count);
            test_vec_std_vec_equality(vec, std_vec);
            idx = (idx + SMALL_PRIME) % (std_vec.size() + 1);
        }
        EXPECT_TRUE(vec.empty());
    }

    TEST_F(vector_test, emplace_back_forward_test) {
        vector<forward_stub> vec;
        vec.emplace_back(SPECIAL_VALUE);
        EXPECT_EQ(vec.back().construct_type,
            forward_stub::constructor_type::int_constructor);
        EXPECT_EQ(vec.back().n, SPECIAL_VALUE);
        vec.emplace_back(0.1f);
        EXPECT_EQ(vec.back().construct_type,
            forward_stub::constructor_type::float_constructor);
        vec.emplace_back(std::string());
        EXPECT_EQ(vec.back().construct_type,
            forward_stub::constructor_type::string_constructor);
    }

    TEST_F(vector_test, emplace_back_return_value_test) {
        vector<forward_stub> vec;
        forward_stub& stub = vec.emplace_back(SPECIAL_VALUE);
        EXPECT_EQ(vec[0].n, SPECIAL_VALUE);
        stub.n++;
        EXPECT_EQ(vec[0].n, SPECIAL_VALUE + 1);
    }

    TEST_F(vector_test, resize_test) {
        vector<constructor_stub> vec;
        vec.resize(SMALL_LIMIT);
        EXPECT_EQ(constructor_stub::default_constructor_invocation_count, SMALL_LIMIT);
        EXPECT_EQ(vec.size(), SMALL_LIMIT);
    }

    TEST_F(vector_test, swap_test) {
        vector<constructor_stub> vec(SMALL_LIMIT);
        std::vector<constructor_stub> std_vec(vec.begin(), vec.end());
        vector<constructor_stub> vec2(MEDIUM_LIMIT);
        std::vector<constructor_stub> std_vec2(vec2.begin(), vec2.end());
        vec.swap(vec2);
        test_vec_std_vec_equality(vec, std_vec2);
        test_vec_std_vec_equality(vec2, std_vec);
    }
}
