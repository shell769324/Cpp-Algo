#pragma once
#include <concepts>
#include "gtest/gtest.h"
#include "utility/constructor_stub.h"
#include "utility/common.h"
#include "src/thread_pool_executor/thread_pool_executor.h"

namespace {
    using namespace algo;

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SPECIAL_VALUE2 = 0xdeadbabe;
    static const int LIMIT = 10000;
    static const int MEDIUM_LIMIT = 1000;
    static const int SMALL_LIMIT = 10;
    static const int REPEAT = 20;

    static const int LOOK_UP = 0;
    static const int INSERT = 1;
    static const int DELETE = 2;

    static std::size_t default_constructor_invocation_count;
    static std::size_t copy_constructor_invocation_count;
    static std::size_t move_constructor_invocation_count;

    void mark_constructor_counts() {
        default_constructor_invocation_count = constructor_stub::default_constructor_invocation_count;
        copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
    }

    void check_constructor_counts(int default_delta=0, int copy_delta=0, int move_delta=0) {
        EXPECT_EQ(default_constructor_invocation_count + default_delta, constructor_stub::default_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count + copy_delta, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(move_constructor_invocation_count + move_delta, constructor_stub::move_constructor_invocation_count);
        mark_constructor_counts();
    }

    template <typename T>
    class map_test : public testing::Test {
        public:
        thread_pool_executor executor;
        protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }

        static void equivalence_test(const T& map, std::vector<typename T::value_type>& stub_pairs) {
            for (unsigned i = 0; i < stub_pairs.size(); i++) {
                const constructor_stub& key = stub_pairs[i].first;
                const constructor_stub& value = stub_pairs[i].second;
                EXPECT_TRUE(map.contains(key));
                const auto& pair = *map.find(key);
                EXPECT_EQ(pair.first.id, key.id);
                EXPECT_EQ(pair.second.id, value.id);
            }
            EXPECT_EQ(map.size(), stub_pairs.size());
        }

        template<typename Map>
        static void begin_end_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            auto stub_pairs = get_random_stub_pair_vector(stubs);
            Map map(stub_pairs.begin(), stub_pairs.end());
            std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
            stub_pairs = get_random_stub_pair_vector(stubs);
            int i = 0;
            mark_constructor_counts();
            for (auto& pair : map) {
                EXPECT_EQ(pair.first, stub_pairs[i].first);
                EXPECT_EQ(pair.second, stub_pairs[i].second);
                EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                    std::is_const_v<Map>);
                i++;
            }
            check_constructor_counts();
            EXPECT_EQ(i, MEDIUM_LIMIT);
        }

        template<typename Map>
        static void rbegin_rend_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            auto stub_pairs = get_random_stub_pair_vector(stubs);
            Map map(stub_pairs.begin(), stub_pairs.end());
            int i = 0;
            std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
            stub_pairs = get_random_stub_pair_vector(stubs);
            mark_constructor_counts();
            for (auto it = map.rbegin(); it != map.rend(); it++, i++) {
                auto& pair = *it;
                EXPECT_EQ(pair.first, stub_pairs[i].first);
                EXPECT_EQ(pair.second, stub_pairs[i].second);
                EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                    std::is_const_v<Map>);
            }
            EXPECT_EQ(i, MEDIUM_LIMIT);
            check_constructor_counts();
        }

        static void insert_find_test(std::size_t size) {
            T map;
            auto stub_pairs = get_random_stub_pair_vector(size);
            int i = 0;
            unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
            for (auto& pair : stub_pairs) {
                mark_constructor_counts();
                map.insert(pair);
                check_constructor_counts(0, 2, 0);
                EXPECT_EQ(map.size(), i + 1);
                if (i % skip == 0) {
                    for (int j = 0; j <= i; j++) {
                        EXPECT_NE(map.find(stub_pairs[j].first), map.cend());
                        EXPECT_EQ(map[stub_pairs[j].first], stub_pairs[j].second);
                    }
                    EXPECT_TRUE(map.is_valid());
                }
                i++;
            }
        }

        static void erase_by_key(std::size_t size) {
            auto stub_pairs = get_random_stub_pair_vector(size);
            T map(stub_pairs.cbegin(), stub_pairs.cend());
            std::size_t curr_size = size;
            unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
            for (auto& pair : stub_pairs) {
                EXPECT_NE(map.find(pair.first), map.end());
                mark_constructor_counts();
                map.erase(pair.first);
                check_constructor_counts();
                curr_size--;
                EXPECT_EQ(map.find(pair.first), map.cend());
                EXPECT_EQ(map.size(), curr_size);
                if (curr_size % skip == 0) {
                    EXPECT_TRUE(map.is_valid());
                }
            }
            EXPECT_TRUE(map.is_empty());
        }

        static void erase_by_iterator(std::size_t size) {
            auto stub_pairs = get_random_stub_pair_vector(size);
            T map(stub_pairs.cbegin(), stub_pairs.cend());
            std::size_t curr_size = size;
            typename T::iterator it = map.begin();
            unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
            while (!map.is_empty()) {
                typename T::value_type pair = *it;
                EXPECT_NE(map.find(pair.first), map.end());
                mark_constructor_counts();
                it = map.erase(it);
                curr_size--;
                check_constructor_counts();
                EXPECT_EQ(map.find(pair.first), map.cend());
                EXPECT_EQ(map.size(), curr_size);
                if (curr_size % skip == 0) {
                    EXPECT_TRUE(map.is_valid());
                }
            }
            EXPECT_EQ(it, map.end());
        }

        template<typename Map>
        static void max_leq_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            auto stub_pairs = get_random_stub_pair_vector(stubs);
            T src(stub_pairs.cbegin(), stub_pairs.cend());
            std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
            int middle_index = stubs.size() / 2;
            Map& map = src;
            // With existing element
            EXPECT_EQ((*map.max_leq(stubs[middle_index])).first, stubs[middle_index]);
            // With absent element
            int num = (stubs[middle_index].id + stubs[middle_index + 1].id) / 2;
            constructor_stub stub(num);
            auto& pair = *map.max_leq(stub);
            EXPECT_EQ(pair.first, stubs[middle_index]);
            EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                    std::is_const_v<Map>);
            EXPECT_EQ(map.max_leq(stubs.front().id - 1), map.end());
        }

        template <typename Map>
        static void min_geq_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            auto stub_pairs = get_random_stub_pair_vector(stubs);
            T src(stub_pairs.cbegin(), stub_pairs.cend());
            Map& map = src;
            std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
            int middle_index = stubs.size() / 2;
            // With existing element
            EXPECT_EQ((*map.min_geq(stubs[middle_index])).first, stubs[middle_index]);
            // With absent element
            int num = (stubs[middle_index].id + stubs[middle_index - 1].id) / 2;
            constructor_stub stub(num);
            auto& pair = *map.min_geq(stub);
            if (stubs[middle_index - 1].id + 1 == stubs[middle_index].id) {
                EXPECT_EQ(pair.first, stubs[middle_index - 1]);
            } else {
                EXPECT_EQ(pair.first, stubs[middle_index]);
            }
            std::cout << pair.first.id;
            EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                    std::is_const_v<Map>);
            EXPECT_EQ(map.min_geq(stubs.back().id + 1), map.end());
        }

        template<typename Resolver>
        void union_of_test_template(const T& map1, const T& map2, Resolver resolver, 
            bool is_parallel=false) {
            T map_copy1(map1);
            T map_copy2(map2);
            T map;
            mark_constructor_counts();
            if (is_parallel) {
                map = union_of<Resolver>(std::move(map_copy1), std::move(map_copy2), this -> executor, resolver);
            } else {
                map = union_of<Resolver>(std::move(map_copy1), std::move(map_copy2), resolver);
            }
            check_constructor_counts();
            EXPECT_TRUE(map.is_valid());
            // No element is missing
            for (const auto& pair : map1) {
                EXPECT_NE(map.find(pair.first), map.end());
            }
            for (const auto& pair : map2) {
                EXPECT_NE(map.find(pair.first), map.end());
            }
            // No extraneous elements
            for (const auto& pair : map) {
                EXPECT_TRUE(map1.find(pair.first) != map1.end() || map2.find(pair.first) != map2.end());
            }
            // Resolution is correct
            for (const auto& pair1 : map1) {
                if (map2.find(pair1.first) != map2.end()) {
                    const auto& pair2 = *map2.find(pair1.first);
                    const auto& pair = *map.find(pair1.first);
                    if (resolver(pair1, pair2)) {
                        EXPECT_EQ(pair.second, pair1.second);
                    } else {
                        EXPECT_EQ(pair.second, pair2.second);
                    }
                }
            }
        }

        template<typename Resolver>
        void intersection_of_test_template(const T& map1, const T& map2, Resolver resolver, bool is_parallel=false) {
            T map_copy1(map1);
            T map_copy2(map2);
            T map;
            mark_constructor_counts();
            if (is_parallel) {
                map = intersection_of<Resolver>(std::move(map_copy1), std::move(map_copy2), this -> executor, resolver);
            } else {
                map = intersection_of<Resolver>(std::move(map_copy1), std::move(map_copy2), resolver);
            }
            check_constructor_counts();
            EXPECT_TRUE(map.is_valid());
            // No extraneous elements
            for (const auto& pair : map) {
                EXPECT_TRUE(map1.find(pair.first) != map1.end() && map2.find(pair.first) != map2.end());
            }
            // Resolution is correct
            for (const auto& pair1 : map1) {
                if (map2.find(pair1.first) != map2.end()) {
                    const auto& pair2 = *map2.find(pair1.first);
                    EXPECT_NE(map.find(pair1.first), map.end());
                    const auto& pair = *map.find(pair1.first);
                    if (resolver(pair1, pair2)) {
                        EXPECT_EQ(pair.second, pair1.second);
                    } else {
                        EXPECT_EQ(pair.second, pair2.second);
                    }
                }
            }
        }

        void difference_of_test_template(const T& map1, const T& map2, bool is_parallel=false) {
            T map_copy1(map1);
            T map_copy2(map2);
            T map;
            mark_constructor_counts();
            if (is_parallel) {
                map = difference_of(std::move(map_copy1), std::move(map_copy2), this -> executor);
            } else {
                map = difference_of(std::move(map_copy1), std::move(map_copy2));
            }
            check_constructor_counts();
            EXPECT_TRUE(map.is_valid());
            // No extraneous elements
            for (const auto& pair : map) {
                EXPECT_TRUE(map1.find(pair.first) != map1.end() && map2.find(pair.first) == map2.end());
            }
            for (const auto& pair : map1) {
                if (map2.find(pair.first) == map2.end()) {
                    EXPECT_NE(map.find(pair.first), map.end());
                }
            }
        }

        static void print_map(const T& map) {
            std::cout << "Printing map of size " << map.size() << std::endl;
            for (const auto& pair : map) {
                std::cout << pair.first.id << ":" << pair.second.id << std::endl;
            }
        }    
    };

    int do_action_lottery(int find = 2, int insert = 2, int remove = 0) {
        int total = find + insert + remove;
        int num = random_number(0, total);
        if (num < find) {
            return LOOK_UP;
        }
        if (num < find + insert) {
            return INSERT;
        }
        return DELETE;
    }


    TYPED_TEST_SUITE_P(map_test);

    TYPED_TEST_P(map_test, default_constructor_test) {
        TypeParam map;
    }

    TYPED_TEST_P(map_test, comp_constructor_test) {
        constructor_stub_comparator comp(true);
        TypeParam map(comp);
        EXPECT_TRUE(map.key_comp().reverse);
    }

    TYPED_TEST_P(map_test, copy_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam map(comp);
        map.insert(stub_pairs.begin(), stub_pairs.end());
        TypeParam map_copy(map);
        EXPECT_TRUE(map_copy.is_valid());
        EXPECT_TRUE(map_copy.key_comp().reverse);
        this -> equivalence_test(map_copy, stub_pairs);
    }

    TYPED_TEST_P(map_test, move_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam map(comp);
        map.insert(stub_pairs.begin(), stub_pairs.end());
        mark_constructor_counts();
        TypeParam map_copy(std::move(map));
        check_constructor_counts();
        EXPECT_TRUE(map_copy.is_valid());
        EXPECT_TRUE(map_copy.key_comp().reverse);
        this -> equivalence_test(map_copy, stub_pairs);
    }

    TYPED_TEST_P(map_test, assignment_operator_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam map1(comp);
        map1.insert(stub_pairs.begin(), stub_pairs.end());
        TypeParam map2;
        map2 = map1;
        EXPECT_TRUE(map2.is_valid());
        this -> equivalence_test(map2, stub_pairs);
    }

    TYPED_TEST_P(map_test, move_assignment_operator_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam map1(comp);
        map1.insert(stub_pairs.begin(), stub_pairs.end());
        TypeParam map2;
        mark_constructor_counts();
        map2 = std::move(map1);
        check_constructor_counts();
        EXPECT_TRUE(map2.is_valid());
        this -> equivalence_test(map2, stub_pairs);
    }
    
    TYPED_TEST_P(map_test, range_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        TypeParam map(stub_pairs.begin(), stub_pairs.end());
        this -> equivalence_test(map, stub_pairs);
    }

    TYPED_TEST_P(map_test, at_test) {
        TypeParam map;
        constructor_stub stub(SPECIAL_VALUE);
        map.insert(std::make_pair(stub, stub));

        mark_constructor_counts();

        EXPECT_EQ(map.at(SPECIAL_VALUE).id, SPECIAL_VALUE);
        EXPECT_THROW(map.at(SPECIAL_VALUE2), std::out_of_range);

        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, at_const_test) {
        constructor_stub stub(SPECIAL_VALUE);
        TypeParam map;
        map.insert(std::make_pair(stub, stub));
        const TypeParam& const_map = map;

        mark_constructor_counts();

        EXPECT_EQ(const_map.at(SPECIAL_VALUE).id, SPECIAL_VALUE);
        EXPECT_THROW(const_map.at(SPECIAL_VALUE2), std::out_of_range);

        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, subscript_reference_lvalue_test) {
        TypeParam map;
        constructor_stub stub(SPECIAL_VALUE);
        map.insert(std::make_pair(stub, stub));

        mark_constructor_counts();

        EXPECT_EQ(map[stub].id, SPECIAL_VALUE);
        map[stub].id++;
        EXPECT_EQ(map[stub].id, SPECIAL_VALUE + 1);
        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, subscript_missing_element_lvalue_test) {
        TypeParam map;
        constructor_stub stub(SPECIAL_VALUE);

        mark_constructor_counts();

        constructor_stub& default_stub = map[stub];
        check_constructor_counts(1, 1, 0);
        EXPECT_EQ((*map.find(stub)).second, default_stub);
    }

    TYPED_TEST_P(map_test, subscript_reference_rvalue_test) {
        TypeParam map;
        constructor_stub stub(SPECIAL_VALUE);
        map.insert(std::make_pair(stub, stub));

        mark_constructor_counts();

        EXPECT_EQ(map[stub].id, SPECIAL_VALUE);
        map[constructor_stub(stub)].id++;
        EXPECT_EQ(map[stub].id, SPECIAL_VALUE + 1);
        check_constructor_counts(0, 1, 0);
    }

    TYPED_TEST_P(map_test, subscript_missing_element_rvalue_test) {
        TypeParam map;
        mark_constructor_counts();

        constructor_stub& default_stub = map[constructor_stub(SPECIAL_VALUE)];
        check_constructor_counts(1, 0, 1);
        EXPECT_EQ((*map.find(SPECIAL_VALUE)).second, default_stub);
    }

    // Iterator tests

    TYPED_TEST_P(map_test, begin_end_test) {
        this -> template begin_end_test_template<TypeParam>();
    }

    TYPED_TEST_P(map_test, const_begin_end_test) {
        this -> template begin_end_test_template<const TypeParam>();
    }

    TYPED_TEST_P(map_test, cbegin_cend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        const TypeParam map(stub_pairs.begin(), stub_pairs.end());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        stub_pairs = get_random_stub_pair_vector(stubs);
        mark_constructor_counts();
        for (auto it = map.cbegin(); it != map.cend(); it++, i++) {
            auto& pair = *it;
            EXPECT_EQ(pair.first, stub_pairs[i].first);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, rbegin_rend_test) {
        this -> template rbegin_rend_test_template<TypeParam>();
    }

    TYPED_TEST_P(map_test, const_rbegin_rend_test) {
        this -> template rbegin_rend_test_template<const TypeParam>();
    }

    TYPED_TEST_P(map_test, crbegin_crend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        const TypeParam map(stub_pairs.begin(), stub_pairs.end());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        stub_pairs = get_random_stub_pair_vector(stubs);
        mark_constructor_counts();
        for (auto it = map.crbegin(); it != map.crend(); it++, i++) {
            auto& pair = *it;
            EXPECT_EQ(pair.first, stub_pairs[i].first);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, is_empty_test) {
        TypeParam map;
        EXPECT_TRUE(map.is_empty());
        map[SPECIAL_VALUE] = SPECIAL_VALUE;
        EXPECT_FALSE(map.is_empty());
    }

    TYPED_TEST_P(map_test, size_test) {
        TypeParam map;
        EXPECT_EQ(map.size(), 0);
        map[SPECIAL_VALUE] = SPECIAL_VALUE;
        EXPECT_EQ(map.size(), 1);
    }

    TYPED_TEST_P(map_test, clear_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        TypeParam map(stub_pairs.begin(), stub_pairs.end());
        map.clear();
        EXPECT_TRUE(map.is_empty());
    }
    
    // Core methods (find and insert) tests
    TYPED_TEST_P(map_test, const_ref_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        TypeParam map;
        EXPECT_EQ(map.find(stub), map.cend());
        typename TypeParam::value_type pair(stub, stub);
        const typename TypeParam::value_type& const_ref = pair;
        mark_constructor_counts();
        map.insert(const_ref);
        check_constructor_counts(0, 2, 0);
        EXPECT_EQ((*map.find(stub)).first.id, SPECIAL_VALUE);
        EXPECT_EQ((*map.find(stub)).second.id, SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, rvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        TypeParam map;
        EXPECT_EQ(map.find(stub), map.cend());
        std::pair<const constructor_stub, constructor_stub> pair(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair));
        check_constructor_counts(0, 1, 1);
        EXPECT_EQ((*map.find(stub)).first.id, SPECIAL_VALUE);
        EXPECT_EQ((*map.find(stub)).second.id, SPECIAL_VALUE);
        std::pair<const constructor_stub, constructor_stub> pair2(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair2));
        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, template_lvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        TypeParam map;
        EXPECT_EQ(map.find(stub), map.cend());
        std::pair<constructor_stub, constructor_stub> pair = std::make_pair(stub, stub);
        mark_constructor_counts();
        map.insert(pair);
        check_constructor_counts(0, 2, 0);
        EXPECT_EQ((*map.find(stub)).first.id, SPECIAL_VALUE);
        EXPECT_EQ((*map.find(stub)).second.id, SPECIAL_VALUE);
        std::pair<const constructor_stub, constructor_stub> pair2(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair2));
        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, template_rvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        TypeParam map;
        EXPECT_EQ(map.find(stub), map.cend());
        std::pair<constructor_stub, constructor_stub> pair = std::make_pair(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair));
        check_constructor_counts(0, 0, 2);
        EXPECT_EQ((*map.find(stub)).first.id, SPECIAL_VALUE);
        EXPECT_EQ((*map.find(stub)).second.id, SPECIAL_VALUE);
        std::pair<const constructor_stub, constructor_stub> pair2(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair2));
        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, insert_return_value_test) {
        TypeParam map;
        constructor_stub key(SPECIAL_VALUE);
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<const constructor_stub, constructor_stub> pair(key, value);
        std::pair<typename TypeParam::iterator, bool> res = map.insert(pair);
        EXPECT_EQ((*res.first).first, key);
        EXPECT_EQ((*res.first).second, value);
        EXPECT_TRUE(res.second);
        std::pair<typename TypeParam::iterator, bool> res2 = map.insert(pair);
        EXPECT_EQ((*res2.first).first, key);
        EXPECT_EQ((*res2.first).second, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(map.size(), 1);
    }


    TYPED_TEST_P(map_test, insert_find_intermediate_test) {
        this -> insert_find_test(SMALL_LIMIT);
    }

    TYPED_TEST_P(map_test, insert_find_stress_test) {
        this -> insert_find_test(MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, insert_range_test) {
        std::vector<typename TypeParam::value_type> stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        TypeParam map(stub_pairs.begin(), stub_pairs.end());
        for (unsigned i = 0; i < stub_pairs.size(); i++) {
            constructor_stub stub = stub_pairs[i].first;
            auto pair = *map.find(stub);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
        }
        EXPECT_TRUE(map.is_valid());
    }

    TYPED_TEST_P(map_test, emplace_lvalue_test) {
        TypeParam map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        map.emplace(stub1, stub2);
        check_constructor_counts(0, 2, 0);
        EXPECT_EQ(map[stub1], stub2);
    }

    TYPED_TEST_P(map_test, emplace_rvalue_test) {
        TypeParam map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        map.emplace(std::move(stub1), std::move(stub2));
        check_constructor_counts(0, 0, 2);
        EXPECT_EQ(map[stub1], stub2);
    }

    TYPED_TEST_P(map_test, emplace_id_forward_test) {
        TypeParam map;
        mark_constructor_counts();
        map.emplace(std::piecewise_construct, std::tuple(SPECIAL_VALUE), std::tuple(-SPECIAL_VALUE));
        check_constructor_counts();
        EXPECT_EQ(map[SPECIAL_VALUE].id, -SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, emplace_return_test) {
        TypeParam map;
        constructor_stub key(SPECIAL_VALUE);
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<typename TypeParam::iterator, bool> res = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
            std::forward_as_tuple(value));
        EXPECT_EQ((*res.first).first, key);
        EXPECT_EQ((*res.first).second, value);
        EXPECT_TRUE(res.second);
        std::pair<typename TypeParam::iterator, bool> res2 = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
            std::forward_as_tuple(value));
        EXPECT_EQ((*res2.first).first, key);
        EXPECT_EQ((*res2.first).second, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(map.size(), 1);
    }

    TYPED_TEST_P(map_test, try_emplace_lvalue_test) {
        TypeParam map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(stub1, stub2);
        check_constructor_counts(0, 2, 0);
        EXPECT_EQ(map[stub1], stub2);
    }

    TYPED_TEST_P(map_test, try_emplace_rvalue_test) {
        TypeParam map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(stub1, std::move(stub2));
        check_constructor_counts(0, 1, 1);
        EXPECT_EQ(map[stub1], stub2);
    }

    TYPED_TEST_P(map_test, try_emplace_id_forward_test) {
        TypeParam map;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(stub, -SPECIAL_VALUE);
        check_constructor_counts(0, 1, 0);
        EXPECT_EQ(map[SPECIAL_VALUE].id, -SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, rvalue_try_emplace_id_forward_test) {
        TypeParam map;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(std::move(stub), -SPECIAL_VALUE);
        check_constructor_counts(0, 0, 1);
        EXPECT_EQ(map[SPECIAL_VALUE].id, -SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, try_emplace_return_test) {
        TypeParam map;
        constructor_stub key(SPECIAL_VALUE);
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<typename TypeParam::iterator, bool> res = map.try_emplace(key, value);
        EXPECT_EQ((*res.first).first, key);
        EXPECT_EQ((*res.first).second, value);
        EXPECT_TRUE(res.second);
        std::pair<typename TypeParam::iterator, bool> res2 = map.try_emplace(key, value);
        EXPECT_EQ((*res2.first).first, key);
        EXPECT_EQ((*res2.first).second, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(map.size(), 1);
    }

    TYPED_TEST_P(map_test, erase_by_key_basic_test) {
        TypeParam map;
        map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE));
        mark_constructor_counts();
        map.erase(SPECIAL_VALUE);
        check_constructor_counts();
        EXPECT_EQ(map.find(SPECIAL_VALUE), map.cend());
    }

    TYPED_TEST_P(map_test, erase_by_key_return_value_test) {
        TypeParam map;
        map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE));
        EXPECT_EQ(map.erase(SPECIAL_VALUE), 1);
        EXPECT_EQ(map.erase(SPECIAL_VALUE), 0);
    }

    TYPED_TEST_P(map_test, erase_by_key_intermediate_test) {
        this -> erase_by_key(SMALL_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_by_key_stress_test) {
        this -> erase_by_key(MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_by_iterator_basic_test) {
        TypeParam map;
        typename TypeParam::iterator it = map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE)).first;
        mark_constructor_counts();
        map.erase(it);
        check_constructor_counts();
        EXPECT_EQ(map.find(SPECIAL_VALUE), map.cend());
    }

    TYPED_TEST_P(map_test, erase_by_iterator_return_value_test) {
        TypeParam map;
        typename TypeParam::iterator it1 = map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE)).first;
        typename TypeParam::iterator it2 = map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE + 1), std::forward_as_tuple(-SPECIAL_VALUE - 1)).first;
        EXPECT_EQ(map.erase(it1), it2);
        EXPECT_EQ(map.erase(it2), map.end());
    }

    TYPED_TEST_P(map_test, erase_by_iterator_intermediate_test) {
        this -> erase_by_iterator(SMALL_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_by_iterator_stress_test) {
        this -> erase_by_iterator(MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_range_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        TypeParam map(stub_pairs.cbegin(), stub_pairs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        std::size_t trisect1 = stubs.size() / 3;
        std::size_t trisect2 = stubs.size() - stubs.size() / 3;
        auto it1 = map.find(stubs[trisect1]);
        auto it2 = map.find(stubs[trisect2]);
        EXPECT_EQ(map.erase(it1, it2), it2);
        EXPECT_EQ(map.size(), stubs.size() - (trisect2 - trisect1));
        for (std::size_t i = trisect1; i < trisect2; i++) {
            EXPECT_EQ(map.find(stubs[i]), map.end());
        }
        map.erase(it2, map.end());
        EXPECT_EQ(map.size(), trisect1);
        for (std::size_t i = trisect2; i < stubs.size(); i++) {
            EXPECT_EQ(map.find(stubs[i]), map.end());
        }
        map.erase(map.begin(), map.end());
        EXPECT_TRUE(map.is_empty());
    }

    TYPED_TEST_P(map_test, swap_test) {
        auto stub_pairs1 = get_random_stub_pair_vector(MEDIUM_LIMIT);
        TypeParam map1(stub_pairs1.cbegin(), stub_pairs1.cend());
        auto stub_pairs2 = get_random_stub_pair_vector(MEDIUM_LIMIT);
        TypeParam map2(stub_pairs2.cbegin(), stub_pairs2.cend());
        std::swap(map1, map2);
        this -> equivalence_test(map2, stub_pairs1);
        this -> equivalence_test(map1, stub_pairs2);
    }

    TYPED_TEST_P(map_test, contains_test) {
        TypeParam map;
        map[constructor_stub(SPECIAL_VALUE)] = constructor_stub(-SPECIAL_VALUE);
        mark_constructor_counts();
        EXPECT_TRUE(map.contains(SPECIAL_VALUE));
        EXPECT_FALSE(map.contains(-SPECIAL_VALUE));
        check_constructor_counts();
    }

    // Upper bound and lower bound tests
    TYPED_TEST_P(map_test, max_leq_test) {
        this -> template max_leq_test_template<TypeParam>();
    }

    TYPED_TEST_P(map_test, max_leq_const_test) {
        this -> template max_leq_test_template<const TypeParam>();
    }

    TYPED_TEST_P(map_test, min_geq_test) {
        this -> template min_geq_test_template<TypeParam>();
    }

    TYPED_TEST_P(map_test, min_geq_const_test) {
        this -> template min_geq_test_template<const TypeParam>();
    }

    class stub_pair_resolver {
        public:
        using const_stub_pair = const std::pair<const constructor_stub, constructor_stub>;

        stub_pair_resolver(bool is_choose_smaller=true) : is_choose_smaller(is_choose_smaller) { }

        bool operator()(const_stub_pair& pair1, const_stub_pair& pair2) const {
            return (pair1.second.id < pair2.second.id) == is_choose_smaller;
        }

        private:
        bool is_choose_smaller;
    };

    TYPED_TEST_P(map_test, union_of_basic_test) {
        TypeParam map1;
        map1[1] = -1;
        TypeParam map2;
        map2[2] = -2;
        this -> union_of_test_template(map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_basic_parallel_test) {
        TypeParam map1;
        map1[1] = -1;
        TypeParam map2;
        map2[2] = -2;
        this -> union_of_test_template(map1, map2, stub_pair_resolver(), true);
    }

    TYPED_TEST_P(map_test, union_of_conflict_basic_test) {
        TypeParam map1;
        map1[1] = -1;
        TypeParam map2;
        map2[1] = -2;
        this -> union_of_test_template(map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_conflict_basic_parallel_test) {
        TypeParam map1;
        map1[1] = -1;
        TypeParam map2;
        map2[1] = -2;
        this -> union_of_test_template(map1, map2, stub_pair_resolver(), true);
    }

    TYPED_TEST_P(map_test, union_of_empty_test) {
        TypeParam map1;
        map1[0] = 0;
        TypeParam map2;
        this -> union_of_test_template(map1, map2, stub_pair_resolver());
        this -> union_of_test_template(map2, map1, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_intermediate_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        TypeParam map1(stub_pairs.begin(), stub_pairs.begin() + SMALL_LIMIT / 2);
        TypeParam map2(stub_pairs.begin() + SMALL_LIMIT / 2, stub_pairs.end());
        this -> union_of_test_template(map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_intermediate_parallel_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        TypeParam map1(stub_pairs.begin(), stub_pairs.begin() + SMALL_LIMIT / 2);
        TypeParam map2(stub_pairs.begin() + SMALL_LIMIT / 2, stub_pairs.end());
        this -> union_of_test_template(map1, map2, stub_pair_resolver(), true);
    }
    
    TYPED_TEST_P(map_test, union_of_conflict_intermediate_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin(), stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.end());
        this -> union_of_test_template(map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_stress_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        TypeParam map1;
        TypeParam map2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 2; i++) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, union_of_stress_parallel_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        TypeParam map1;
        TypeParam map2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 2; i++) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(map1, map2, stub_pair_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(map_test, union_of_conflict_stress_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; i++) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, union_of_conflict_stress_parallel_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; i++) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(map1, map2, stub_pair_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(map_test, intersection_of_basic_test) {
        TypeParam map1;
        map1[0] = 0;
        map1[1] = -1;
        TypeParam map2;
        map2[0] = 2;
        map2[3] = -3;
        this -> intersection_of_test_template(map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, intersection_of_basic_parallel_test) {
        TypeParam map1;
        map1[0] = 0;
        map1[1] = -1;
        TypeParam map2;
        map2[0] = 2;
        map2[3] = -3;
        this -> intersection_of_test_template(map1, map2, stub_pair_resolver(), true);
    }

    TYPED_TEST_P(map_test, intersection_of_empty_test) {
        TypeParam map1;
        map1[0] = 0;
        TypeParam map2;
        map2[1] = -1;
        this -> intersection_of_test_template(map1, map2, stub_pair_resolver());
        this -> intersection_of_test_template(map2, map1, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, intersection_of_intermediate_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; i++) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, intersection_of_stress_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; i++) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, intersection_of_stress_parallel_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; i++) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(map1, map2, stub_pair_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(map_test, difference_of_basic_test) {
        TypeParam map1;
        map1[0] = 0;
        map1[1] = -1;
        TypeParam map2;
        map2[0] = 2;
        map1[2] = -2;
        this -> difference_of_test_template(map1, map2);
    }

    TYPED_TEST_P(map_test, difference_of_basic_parallel_test) {
        TypeParam map1;
        map1[0] = 0;
        map1[1] = -1;
        TypeParam map2;
        map2[0] = 2;
        map1[2] = -2;
        this -> difference_of_test_template(map1, map2, true);
    }

    TYPED_TEST_P(map_test, difference_of_empty_test) {
        TypeParam map1;
        map1[0] = 0;
        TypeParam map2;
        map2[0] = -1;
        this -> difference_of_test_template(map1, map2);
        this -> difference_of_test_template(map2, map1);
    }

    TYPED_TEST_P(map_test, difference_of_intermediate_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; i++) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(map1, map2);
            }
        }
    }

    TYPED_TEST_P(map_test, difference_of_stress_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; i++) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(map1, map2);
            }
        }
    }

    TYPED_TEST_P(map_test, difference_of_stress_parallel_test) {
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        TypeParam map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        TypeParam map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; i++) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(map1, map2, true);
            }
        }
    }

    // Mixed stress tests
    TYPED_TEST_P(map_test, mixed_stress_test) {
        TypeParam map;
        int SRTESS_LIMIT = 20000;

        std::map<constructor_stub, constructor_stub, constructor_stub_comparator> stub_map;
        for (int i = 0; i < SRTESS_LIMIT; i++) {
            int num = random_number(0, 4000);
            switch (do_action_lottery()) {
                case LOOK_UP: {
                    if (stub_map.contains(num) && map.contains(num)) {
                        EXPECT_EQ(stub_map[num], map[num]);
                    } else {
                        EXPECT_EQ(stub_map.contains(num), map.contains(num));
                    }
                    break;
                }
                case INSERT: {
                    auto pair = std::make_pair(constructor_stub(num), constructor_stub(-num));
                    auto expect_res = stub_map.insert(pair);
                    auto actual_res = map.insert(pair);
                    if (expect_res.first != stub_map.end() && actual_res.first != map.end()) {
                        auto expected = *expect_res.first;
                        auto actual = *actual_res.first;
                        EXPECT_EQ(expected.first, actual.first);
                        EXPECT_EQ(expected.second, actual.second);
                    } else {
                        EXPECT_EQ(expect_res.first != stub_map.end(), actual_res.first != map.end());
                    }
                    break;
                }
                case DELETE: {
                    EXPECT_EQ(stub_map.erase(num), map.erase(num));
                    break;
                }
            }
        }
    }

    REGISTER_TYPED_TEST_SUITE_P(map_test,
        default_constructor_test,
        comp_constructor_test,
        copy_constructor_test,
        move_constructor_test,
        assignment_operator_test,
        move_assignment_operator_test,
        range_constructor_test,     
        at_test,
        at_const_test,
        subscript_reference_lvalue_test,
        subscript_missing_element_lvalue_test,
        subscript_reference_rvalue_test,
        subscript_missing_element_rvalue_test,
        begin_end_test,
        const_begin_end_test,
        cbegin_cend_test,
        rbegin_rend_test,
        const_rbegin_rend_test,
        crbegin_crend_test,
        is_empty_test,
        size_test,
        clear_test,
        const_ref_insert_find_basic_test,
        rvalue_insert_find_basic_test,
        template_lvalue_insert_find_basic_test,
        template_rvalue_insert_find_basic_test,
        insert_return_value_test,
        insert_find_intermediate_test,
        insert_find_stress_test,
        insert_range_test,
        emplace_lvalue_test,
        emplace_rvalue_test,
        emplace_id_forward_test,
        emplace_return_test,
        try_emplace_lvalue_test,
        try_emplace_rvalue_test,
        try_emplace_id_forward_test,
        rvalue_try_emplace_id_forward_test,
        try_emplace_return_test,
        erase_by_key_basic_test,
        erase_by_key_return_value_test,
        erase_by_key_intermediate_test,
        erase_by_key_stress_test,
        erase_by_iterator_basic_test,
        erase_by_iterator_return_value_test,
        erase_by_iterator_intermediate_test,
        erase_by_iterator_stress_test,
        erase_range_test,
        swap_test,
        contains_test,
        max_leq_test,
        max_leq_const_test,
        min_geq_test,
        min_geq_const_test,
        union_of_basic_test,
        union_of_basic_parallel_test,
        union_of_conflict_basic_test,
        union_of_conflict_basic_parallel_test,
        union_of_empty_test,
        union_of_intermediate_test,
        union_of_intermediate_parallel_test,
        union_of_conflict_intermediate_test,
        union_of_stress_test,
        union_of_stress_parallel_test,
        union_of_conflict_stress_test,
        union_of_conflict_stress_parallel_test,
        intersection_of_basic_test,
        intersection_of_basic_parallel_test,
        intersection_of_empty_test,
        intersection_of_intermediate_test,
        intersection_of_stress_test,
        intersection_of_stress_parallel_test,
        difference_of_basic_test,
        difference_of_basic_parallel_test,
        difference_of_empty_test,
        difference_of_intermediate_test,
        difference_of_stress_test,
        difference_of_stress_parallel_test,
        mixed_stress_test
    );
}
