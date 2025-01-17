#pragma once
#include <concepts>
#include "gtest/gtest.h"
#include "utility/constructor_stub.h"
#include "utility/common.h"
#include "src/thread_pool_executor/thread_pool_executor.h"
#include "utility/tracking_allocator.h"
#include "utility/construction_destruction_tracker.h"

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
        construction_destruction_tracker tracker;
        typename T::regular_type::allocator_type allocator;
        protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            tracking_allocator<typename T::regular_type::node_type>::reset();
            tracking_allocator<typename T::copy_only_type::node_type>::reset();
            tracking_allocator<typename T::move_only_type::node_type>::reset();
            tracker.reset();
        }

        virtual void TearDown() {
            tracker.check();
            tracking_allocator<typename T::regular_type::node_type>::check();
            tracking_allocator<typename T::copy_only_type::node_type>::check();
            tracking_allocator<typename T::move_only_type::node_type>::check();
        }
    };

    template<typename Map>
    void equivalence_test(const Map& map, std::vector<typename Map::value_type>& stub_pairs) {
        for (unsigned i = 0; i < stub_pairs.size(); ++i) {
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
    void begin_end_test_template() {
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
            ++i;
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    template<typename Map>
    void rbegin_rend_test_template(construction_destruction_tracker& tracker) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        tracker.mark();
        Map map(stub_pairs.begin(), stub_pairs.end());
        tracker.check_marked();
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        stub_pairs = get_random_stub_pair_vector(stubs);
        mark_constructor_counts();
        for (auto it = map.rbegin(); it != map.rend(); ++it, ++i) {
            auto& pair = *it;
            EXPECT_EQ(pair.first, stub_pairs[i].first);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
            EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                std::is_const_v<Map>);
        }
        EXPECT_EQ(i, MEDIUM_LIMIT);
        check_constructor_counts();
    }

    template<typename Map>
    void insert_find_test(construction_destruction_tracker& tracker, std::size_t size) {
        Map map;
        auto stub_pairs = get_random_stub_pair_vector(size);
        int i = 0;
        unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
        for (auto& pair : stub_pairs) {
            mark_constructor_counts();
            tracker.mark();
            map.insert(pair);
            tracker.check_marked();
            check_constructor_counts(0, 2, 0);
            EXPECT_EQ(map.size(), i + 1);
            if (i % skip == 0) {
                for (int j = 0; j <= i; ++j) {
                    EXPECT_NE(map.find(stub_pairs[j].first), map.cend());
                    EXPECT_EQ(map[stub_pairs[j].first], stub_pairs[j].second);
                }
                EXPECT_TRUE(map.__is_valid());
            }
            ++i;
        }
    }

    template<typename Map>
    void erase_by_key(construction_destruction_tracker& tracker, std::size_t size) {
        auto stub_pairs = get_random_stub_pair_vector(size);
        Map map(stub_pairs.cbegin(), stub_pairs.cend());
        std::size_t curr_size = size;
        unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
        for (auto& pair : stub_pairs) {
            EXPECT_NE(map.find(pair.first), map.end());
            EXPECT_EQ(map.count(pair.first), 1);
            mark_constructor_counts();
            tracker.mark();
            map.erase(pair.first);
            tracker.check_marked();
            check_constructor_counts();
            --curr_size;
            EXPECT_EQ(map.find(pair.first), map.cend());
            EXPECT_EQ(map.count(pair.first), 0);
            EXPECT_EQ(map.size(), curr_size);
            if (curr_size % skip == 0) {
                EXPECT_TRUE(map.__is_valid());
            }
        }
        EXPECT_TRUE(map.empty());
    }

    template<typename Map>
    void erase_by_iterator(construction_destruction_tracker& tracker, std::size_t size) {
        auto stub_pairs = get_random_stub_pair_vector(size);
        Map map(stub_pairs.cbegin(), stub_pairs.cend());
        std::size_t curr_size = size;
        typename Map::iterator it = map.begin();
        unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
        while (!map.empty()) {
            typename Map::value_type pair = *it;
            EXPECT_NE(map.find(pair.first), map.end());
            mark_constructor_counts();
            tracker.mark();
            it = map.erase(it);
            tracker.check_marked();
            --curr_size;
            check_constructor_counts();
            EXPECT_EQ(map.find(pair.first), map.cend());
            EXPECT_EQ(map.size(), curr_size);
            if (curr_size % skip == 0) {
                EXPECT_TRUE(map.__is_valid());
            }
        }
        EXPECT_EQ(it, map.end());
    }

    template<typename Map>
    static void upper_bound_test_template() {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT, 0, LIMIT);
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        for (auto& stub : stubs) {
            stub.id *= 2;
        }
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        using raw_type = std::remove_const_t<Map>;
        raw_type src(stub_pairs.cbegin() + 1, stub_pairs.cend());
        Map& map = src;
        // Query an element that is less than the smallest in the map
        EXPECT_EQ((*map.upper_bound(stubs.front())).first, stubs[1]);
        for (std::size_t i = 1; i + 1 < stubs.size(); ++i) {
            // With existing element
            EXPECT_EQ((*map.upper_bound(stubs[i])).first, stubs[i + 1]);
            // With absent element
            int num = (stubs[i].id + stubs[i + 1].id) / 2;
            constructor_stub stub(num);
            auto& pair = *map.upper_bound(stub);
            EXPECT_EQ(pair.first, stubs[i + 1]);
        }
        EXPECT_EQ(map.upper_bound(stubs.back()), map.end());
        auto& pair = *map.upper_bound(stubs.front());
        EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                std::is_const_v<Map>);
    }

    template <typename Map>
    static void lower_bound_test_template() {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        for (auto& stub : stubs) {
            stub.id *= 2;
        }
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        using raw_type = std::remove_const_t<Map>;
        raw_type src(stub_pairs.cbegin() + 1, stub_pairs.cend());
        Map& map = src;
        // Query an element that is less than the smallest in the map
        EXPECT_EQ((*map.lower_bound(stubs.front())).first, stubs[1]);
        for (std::size_t i = 1; i + 1 < stubs.size(); ++i) {
            // With existing element
            EXPECT_EQ((*map.lower_bound(stubs[i])).first, stubs[i]);
            // With absent element
            int num = (stubs[i].id + stubs[i + 1].id) / 2;
            constructor_stub stub(num);
            auto& pair = *map.lower_bound(stub);
            EXPECT_EQ(pair.first, stubs[i + 1]);
        }
        EXPECT_EQ(map.lower_bound(stubs.back().id + 1), map.end());
        auto& pair = *map.upper_bound(stubs.front());
        EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(pair)>>,
                std::is_const_v<Map>);
    }

    template<typename Map, typename Resolver>
    void union_of_test_template(construction_destruction_tracker& tracker, thread_pool_executor& executor, 
                                const Map& map1, const Map& map2, Resolver resolver, 
        bool is_parallel=false) {
        Map map_copy1(map1);
        Map map_copy2(map2);
        Map map;
        mark_constructor_counts();
        tracker.mark();
        if (is_parallel) {
            map = union_of<Resolver>(std::move(map_copy1), std::move(map_copy2), executor, resolver);
        } else {
            map = union_of<Resolver>(std::move(map_copy1), std::move(map_copy2), resolver);
        }
        tracker.check_marked();
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
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

    template<typename Map, typename Resolver>
    void intersection_of_test_template(construction_destruction_tracker& tracker, thread_pool_executor& executor, 
                                       const Map& map1, const Map& map2, Resolver resolver, bool is_parallel=false) {
        Map map_copy1(map1);
        Map map_copy2(map2);
        Map map;
        mark_constructor_counts();
        tracker.mark();
        if (is_parallel) {
            map = intersection_of<Resolver>(std::move(map_copy1), std::move(map_copy2), executor, resolver);
        } else {
            map = intersection_of<Resolver>(std::move(map_copy1), std::move(map_copy2), resolver);
        }
        tracker.check_marked();
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
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

    template<typename Map>
    void difference_of_test_template(thread_pool_executor& executor, const Map& map1, const Map& map2, bool is_parallel=false) {
        Map map_copy1(map1);
        Map map_copy2(map2);
        Map map;
        mark_constructor_counts();
        if (is_parallel) {
            map = difference_of(std::move(map_copy1), std::move(map_copy2), executor);
        } else {
            map = difference_of(std::move(map_copy1), std::move(map_copy2));
        }
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
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

    TYPED_TEST_P(map_test, trait_test) {
        using Map = typename TypeParam::regular_type;
        using const_node_type = constify<std::pair<const constructor_stub, constructor_stub>, typename Map::node_type>;
        bool correct = std::is_same_v<typename Map::key_type, constructor_stub>
            && std::is_same_v<typename Map::mapped_type, constructor_stub>
            && std::is_same_v<typename Map::value_type, std::pair<const constructor_stub, constructor_stub> >
            && std::is_same_v<typename Map::size_type, std::size_t>
            && std::is_same_v<typename Map::difference_type, std::ptrdiff_t>
            && std::is_same_v<typename Map::key_compare, constructor_stub_comparator>
            && std::is_same_v<typename Map::allocator_type, tracking_allocator<std::pair<const constructor_stub, constructor_stub> > >
            && std::is_same_v<typename Map::reference, std::pair<const constructor_stub, constructor_stub>&>
            && std::is_same_v<typename Map::const_reference, const std::pair<const constructor_stub, constructor_stub>&>
            && std::is_same_v<typename Map::pointer, std::pair<const constructor_stub, constructor_stub>*>
            && std::is_same_v<typename Map::const_pointer, const std::pair<const constructor_stub, constructor_stub>*>
            && std::is_same_v<typename Map::iterator, binary_tree_iterator<typename Map::node_type> >
            && std::is_same_v<typename Map::const_iterator, binary_tree_iterator<const_node_type> >
            && std::is_same_v<typename Map::reverse_iterator, std::reverse_iterator<binary_tree_iterator<typename Map::node_type> > >
            && std::is_same_v<typename Map::const_reverse_iterator, std::reverse_iterator<binary_tree_iterator<const_node_type> > >;
        EXPECT_TRUE(correct);
    }

    template <typename Map>
    void default_constructor_test_helper(Map& map) {
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(allocated<typename Map::node_type>, 1);
    }

    TYPED_TEST_P(map_test, default_constructor_test) {
        typename TypeParam::regular_type map;
        default_constructor_test_helper(map);
    }

    TYPED_TEST_P(map_test, allocator_constructor_test) {
        typename TypeParam::regular_type map(this -> allocator);
        default_constructor_test_helper(map);
        EXPECT_EQ(this -> allocator, map.get_allocator());
    }

    template <typename Map, typename Comp>
    void comp_allocator_constructor_test_helper(Map& map, Comp& comp) {
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(allocated<typename Map::node_type>, 1);

        EXPECT_EQ(comp, map.key_comp());
    }

    TYPED_TEST_P(map_test, comp_allocator_constructor_test) {
        constructor_stub_comparator comp(true);
        typename TypeParam::regular_type map(comp, this -> allocator);
        comp_allocator_constructor_test_helper(map, comp);
        EXPECT_EQ(this -> allocator, map.get_allocator());
    }

    TYPED_TEST_P(map_test, comp_allocator_constructor_optional_test) {
        constructor_stub_comparator comp(true);
        typename TypeParam::regular_type map(comp);
        comp_allocator_constructor_test_helper(map, comp);
    }

    template <typename Map, typename Pairs>
    void comp_allocator_range_constructor_test_helper(Map& map, Pairs& pairs) {
        equivalence_test(map, pairs);
        EXPECT_EQ(allocated<typename Map::node_type>, pairs.size() + 1);
    }

    TYPED_TEST_P(map_test, comp_allocator_range_constructor_test) {
        constructor_stub_comparator comp;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end(), comp, this -> allocator);
        comp_allocator_range_constructor_test_helper(map, stub_pairs);
        EXPECT_EQ(comp, map.key_comp());
        EXPECT_EQ(this -> allocator, map.get_allocator());
    }

    TYPED_TEST_P(map_test, comp_allocator_range_constructor_optional_allocator_test) {
        constructor_stub_comparator comp;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end(), comp);
        comp_allocator_range_constructor_test_helper(map, stub_pairs);
        EXPECT_EQ(comp, map.key_comp());
    }

    TYPED_TEST_P(map_test, comp_allocator_range_constructor_both_optional_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end());
        comp_allocator_range_constructor_test_helper(map, stub_pairs);
    }

    TYPED_TEST_P(map_test, allocator_range_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end(), this -> allocator);
        comp_allocator_range_constructor_test_helper(map, stub_pairs);
        EXPECT_EQ(this -> allocator, map.get_allocator());
    }

    TYPED_TEST_P(map_test, copy_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        using Map = typename TypeParam::regular_type;
        constructor_stub_comparator comp(true);
        Map map(comp);
        map.insert(stub_pairs.begin(), stub_pairs.end());
        std::size_t alloc_count = allocated<typename Map::node_type>;
        Map map_copy(map);
        EXPECT_EQ(alloc_count + SMALL_LIMIT + 1, allocated<typename Map::node_type>);
        EXPECT_TRUE(map_copy.__is_valid());
        EXPECT_EQ(map_copy.key_comp(), comp);
        equivalence_test(map_copy, stub_pairs);
        EXPECT_EQ(map.get_allocator(), map_copy.get_allocator());
    }

    TYPED_TEST_P(map_test, allocator_copy_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Map = typename TypeParam::regular_type;
        Map map(comp);
        map.insert(stub_pairs.begin(), stub_pairs.end());
        std::size_t alloc_count = allocated<typename Map::node_type>;
        Map map_copy(map, this -> allocator);
        EXPECT_EQ(alloc_count + SMALL_LIMIT + 1, allocated<typename Map::node_type>);
        EXPECT_TRUE(map_copy.__is_valid());
        EXPECT_EQ(map_copy.key_comp(), comp);
        equivalence_test(map_copy, stub_pairs);
        EXPECT_EQ(this -> allocator, map_copy.get_allocator());
    }

    TYPED_TEST_P(map_test, move_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Map = typename TypeParam::regular_type;
        Map map(comp, this -> allocator);
        map.insert(stub_pairs.begin(), stub_pairs.end());
        mark_constructor_counts();
        std::size_t alloc_count = allocated<typename Map::node_type>;
        Map map_copy(std::move(map));
        EXPECT_EQ(alloc_count, allocated<typename Map::node_type>);
        check_constructor_counts();
        EXPECT_TRUE(map_copy.__is_valid());
        EXPECT_EQ(map_copy.key_comp(), comp);
        equivalence_test(map_copy, stub_pairs);
        EXPECT_EQ(this -> allocator, map_copy.get_allocator());
    }

    TYPED_TEST_P(map_test, allocator_move_constructor_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Map = typename TypeParam::regular_type;
        Map map(comp);
        map.insert(stub_pairs.begin(), stub_pairs.end());
        mark_constructor_counts();
        std::size_t alloc_count = allocated<typename Map::node_type>;
        Map map_copy(std::move(map), this -> allocator);
        EXPECT_EQ(alloc_count, allocated<typename Map::node_type>);
        check_constructor_counts();
        EXPECT_TRUE(map_copy.__is_valid());
        EXPECT_EQ(map_copy.key_comp(), comp);
        equivalence_test(map_copy, stub_pairs);
        EXPECT_EQ(this -> allocator, map_copy.get_allocator());
    }

    TYPED_TEST_P(map_test, assignment_operator_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Map = typename TypeParam::regular_type;
        Map map1(comp);
        map1.insert(stub_pairs.begin(), stub_pairs.end());
        Map map2;
        map2 = map1;
        EXPECT_TRUE(map2.__is_valid());
        equivalence_test(map2, stub_pairs);
    }

    TYPED_TEST_P(map_test, move_assignment_operator_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Map = typename TypeParam::regular_type;
        Map map1(comp);
        map1.insert(stub_pairs.begin(), stub_pairs.end());
        Map map2;
        mark_constructor_counts();
        map2 = std::move(map1);
        check_constructor_counts();
        EXPECT_TRUE(map2.__is_valid());
        equivalence_test(map2, stub_pairs);
    }

    TYPED_TEST_P(map_test, at_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub(SPECIAL_VALUE);
        map.insert(std::make_pair(stub, stub));

        mark_constructor_counts();

        EXPECT_EQ(map.at(SPECIAL_VALUE).id, SPECIAL_VALUE);
        EXPECT_THROW(map.at(SPECIAL_VALUE2), std::out_of_range);

        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, at_const_test) {
        constructor_stub stub(SPECIAL_VALUE);
        using Map = typename TypeParam::regular_type;
        Map map;
        map.insert(std::make_pair(stub, stub));
        const Map& const_map = map;

        mark_constructor_counts();

        EXPECT_EQ(const_map.at(SPECIAL_VALUE).id, SPECIAL_VALUE);
        EXPECT_THROW(const_map.at(SPECIAL_VALUE2), std::out_of_range);

        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, subscript_reference_lvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub(SPECIAL_VALUE);
        map.insert(std::make_pair(stub, stub));

        mark_constructor_counts();

        EXPECT_EQ(map[stub].id, SPECIAL_VALUE);
        EXPECT_TRUE(map.__is_valid());
        map[stub].id++;
        EXPECT_EQ(map[stub].id, SPECIAL_VALUE + 1);
        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, subscript_missing_element_lvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub(SPECIAL_VALUE);

        mark_constructor_counts();

        constructor_stub& default_stub = map[stub];
        EXPECT_TRUE(map.__is_valid());
        check_constructor_counts(1, 1, 0);
        EXPECT_EQ((*map.find(stub)).second, default_stub);
    }

    TYPED_TEST_P(map_test, subscript_reference_rvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub(SPECIAL_VALUE);
        map.insert(std::make_pair(stub, stub));

        mark_constructor_counts();

        EXPECT_EQ(map[stub].id, SPECIAL_VALUE);
        map[constructor_stub(stub)].id++;
        EXPECT_EQ(map[stub].id, SPECIAL_VALUE + 1);
        check_constructor_counts(0, 1, 0);
    }

    TYPED_TEST_P(map_test, subscript_missing_element_rvalue_test) {
        typename TypeParam::regular_type map;
        mark_constructor_counts();

        constructor_stub& default_stub = map[constructor_stub(SPECIAL_VALUE)];
        check_constructor_counts(1, 0, 1);
        EXPECT_EQ((*map.find(SPECIAL_VALUE)).second, default_stub);
    }

    // Iterator tests

    TYPED_TEST_P(map_test, begin_end_test) {
        begin_end_test_template<typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(map_test, const_begin_end_test) {
        begin_end_test_template<const typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(map_test, cbegin_cend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        const typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        stub_pairs = get_random_stub_pair_vector(stubs);
        mark_constructor_counts();
        for (auto it = map.cbegin(); it != map.cend(); ++it, ++i) {
            auto& pair = *it;
            EXPECT_EQ(pair.first, stub_pairs[i].first);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, rbegin_rend_test) {
        rbegin_rend_test_template<typename TypeParam::regular_type>(this -> tracker);
    }

    TYPED_TEST_P(map_test, const_rbegin_rend_test) {
        rbegin_rend_test_template<const typename TypeParam::regular_type>(this -> tracker);
    }

    TYPED_TEST_P(map_test, crbegin_crend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        const typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        stub_pairs = get_random_stub_pair_vector(stubs);
        mark_constructor_counts();
        for (auto it = map.crbegin(); it != map.crend(); ++it, ++i) {
            auto& pair = *it;
            EXPECT_EQ(pair.first, stub_pairs[i].first);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, empty_test) {
        typename TypeParam::regular_type map;
        EXPECT_TRUE(map.empty());
        map[SPECIAL_VALUE] = SPECIAL_VALUE;
        EXPECT_FALSE(map.empty());
    }

    TYPED_TEST_P(map_test, size_test) {
        typename TypeParam::regular_type map;
        EXPECT_EQ(map.size(), 0);
        map[SPECIAL_VALUE] = SPECIAL_VALUE;
        EXPECT_EQ(map.size(), 1);
    }

    TYPED_TEST_P(map_test, clear_test) {
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        typename TypeParam::regular_type map(stub_pairs.begin(), stub_pairs.end());
        this -> tracker.mark();
        map.clear();
        this -> tracker.check_marked();
        EXPECT_TRUE(map.empty());
        EXPECT_TRUE(map.__is_valid());
    }
    
    // Core methods (find and insert) tests
    TYPED_TEST_P(map_test, const_ref_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        using Map = typename TypeParam::regular_type;
        Map map;
        EXPECT_EQ(map.find(stub), map.cend());
        typename Map::value_type pair(stub, stub);
        const typename Map::value_type& const_ref = pair;
        mark_constructor_counts();
        map.insert(const_ref);
        check_constructor_counts(0, 2, 0);
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ((*map.find(stub)).first.id, SPECIAL_VALUE);
        EXPECT_EQ((*map.find(stub)).second.id, SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, rvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        typename TypeParam::regular_type map;
        EXPECT_EQ(map.find(stub), map.cend());
        std::pair<const constructor_stub, constructor_stub> pair(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair));
        check_constructor_counts(0, 1, 1);
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ((*map.find(stub)).first.id, SPECIAL_VALUE);
        EXPECT_EQ((*map.find(stub)).second.id, SPECIAL_VALUE);
        std::pair<const constructor_stub, constructor_stub> pair2(stub, stub);
        mark_constructor_counts();
        map.insert(std::move(pair2));
        check_constructor_counts();
    }

    TYPED_TEST_P(map_test, template_lvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        typename TypeParam::regular_type map;
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
        typename TypeParam::regular_type map;
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
        using Map = typename TypeParam::regular_type;
        Map map;
        constructor_stub key(SPECIAL_VALUE);
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<const constructor_stub, constructor_stub> pair(key, value);
        std::pair<typename Map::iterator, bool> res = map.insert(pair);
        EXPECT_EQ((*res.first).first, key);
        EXPECT_EQ((*res.first).second, value);
        EXPECT_TRUE(res.second);
        std::pair<typename Map::iterator, bool> res2 = map.insert(pair);
        EXPECT_EQ((*res2.first).first, key);
        EXPECT_EQ((*res2.first).second, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(map.size(), 1);
    }


    TYPED_TEST_P(map_test, insert_find_intermediate_test) {
        insert_find_test<typename TypeParam::regular_type>(this -> tracker, SMALL_LIMIT);
    }

    TYPED_TEST_P(map_test, insert_find_stress_test) {
        insert_find_test<typename TypeParam::regular_type>(this -> tracker, MEDIUM_LIMIT);
    }

    template<typename Map>
    void insert_hint_equal_test_helper(int hint_delta) {
        Map map;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            map.emplace(std::piecewise_construct, std::tuple(i), std::tuple(i));
        }
        int val = SMALL_LIMIT / 2;
        constructor_stub key(val);
        constructor_stub value(-val);
        auto it = map.insert(std::next(map.begin(), SMALL_LIMIT / 2 + hint_delta), 
            typename Map::value_type(key, value));
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(it -> first, val);
        EXPECT_EQ(it -> second, val);
    }

    TYPED_TEST_P(map_test, insert_hint_equal_hint_test) {
        insert_hint_equal_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(map_test, insert_hint_equal_prev_hint_test) {
        insert_hint_equal_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(map_test, insert_hint_equal_next_hint_test) {
        insert_hint_equal_test_helper<typename TypeParam::regular_type>(1);
    }

    template<typename Map>
    void insert_hint_success_test_helper(int hint_delta) {
        Map map;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            map.emplace(std::piecewise_construct, std::tuple(i), std::tuple(i));
        }
        map.erase(SMALL_LIMIT / 2);
        int val = SMALL_LIMIT / 2;
        constructor_stub key(val);
        constructor_stub value(-val);
        auto it = map.insert(std::next(map.begin(), SMALL_LIMIT / 2 + hint_delta), typename Map::value_type(key, value));
        EXPECT_TRUE(map.contains(SMALL_LIMIT / 2));
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(it -> first, val);
        EXPECT_EQ(it -> second, -val);
    }

    TYPED_TEST_P(map_test, insert_hint_prev_bound_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(map_test, insert_hint_overshoot_hint_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(1);
    }

    TYPED_TEST_P(map_test, insert_hint_right_bound_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(map_test, insert_hint_undershoot_hint_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(-2);
    }

    TYPED_TEST_P(map_test, insert_hint_begin_hint_test) {
        typename TypeParam::regular_type map;
        map.emplace(std::piecewise_construct, std::tuple(1), std::tuple(1));
        constructor_stub key(0);
        constructor_stub value(0);
        map.insert(map.begin(), std::make_pair(key, value));
        EXPECT_EQ(0, map[0]);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, insert_hint_end_hint_test) {
        typename TypeParam::regular_type map;
        map.emplace(std::piecewise_construct, std::tuple(0), std::tuple(0));
        constructor_stub key(1);
        constructor_stub value(1);
        map.insert(map.begin(), std::make_pair(key, value));
        EXPECT_EQ(1, map[1]);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, insert_range_test) {
        using Map = typename TypeParam::regular_type;
        std::vector<typename Map::value_type> stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        Map map(stub_pairs.begin(), stub_pairs.end());
        for (unsigned i = 0; i < stub_pairs.size(); ++i) {
            constructor_stub stub = stub_pairs[i].first;
            auto pair = *map.find(stub);
            EXPECT_EQ(pair.second, stub_pairs[i].second);
        }
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, emplace_lvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        this -> tracker.mark();
        map.emplace(stub1, stub2);
        this -> tracker.check_marked();
        check_constructor_counts(0, 2, 0);
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(map[stub1], stub2);
    }

    TYPED_TEST_P(map_test, emplace_rvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        this -> tracker.mark();
        map.emplace(std::move(stub1), std::move(stub2));
        this -> tracker.check_marked();
        check_constructor_counts(0, 0, 2);
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(map[stub1], stub2);
    }

    TYPED_TEST_P(map_test, emplace_id_forward_test) {
        typename TypeParam::regular_type map;
        mark_constructor_counts();
        this -> tracker.mark();
        map.emplace(std::piecewise_construct, std::tuple(SPECIAL_VALUE), std::tuple(-SPECIAL_VALUE));
        this -> tracker.check_marked();
        check_constructor_counts();
        EXPECT_EQ(map[SPECIAL_VALUE].id, -SPECIAL_VALUE);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, emplace_return_test) {
        using Map = typename TypeParam::regular_type;
        Map map;
        constructor_stub key(SPECIAL_VALUE);
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<typename Map::iterator, bool> res = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
            std::forward_as_tuple(value));
        EXPECT_EQ((*res.first).first, key);
        EXPECT_EQ((*res.first).second, value);
        EXPECT_TRUE(res.second);
        std::pair<typename Map::iterator, bool> res2 = map.emplace(std::piecewise_construct, std::forward_as_tuple(key),
            std::forward_as_tuple(value));
        EXPECT_EQ((*res2.first).first, key);
        EXPECT_EQ((*res2.first).second, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(map.size(), 1);
    }

    template<typename Map>
    void emplace_hint_equal_test_helper(int hint_delta) {
        Map map;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            map.emplace(std::piecewise_construct, std::tuple(i), std::tuple(i));
        }
        mark_constructor_counts();
        int val = SMALL_LIMIT / 2;
        map.emplace_hint(std::next(map.begin(), SMALL_LIMIT / 2 + hint_delta), std::piecewise_construct, std::tuple(val), std::tuple(-val));
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, emplace_hint_equal_hint_test) {
        emplace_hint_equal_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(map_test, emplace_hint_equal_prev_hint_test) {
        emplace_hint_equal_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(map_test, emplace_hint_equal_next_hint_test) {
        emplace_hint_equal_test_helper<typename TypeParam::regular_type>(1);
    }

    template<typename Map>
    void emplace_hint_success_test_helper(int hint_delta) {
        Map map;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            map.emplace(std::piecewise_construct, std::tuple(i), std::tuple(i));
        }
        map.erase(SMALL_LIMIT / 2);
        mark_constructor_counts();
        int val = SMALL_LIMIT / 2;
        map.emplace_hint(std::next(map.begin(), SMALL_LIMIT / 2 + hint_delta), std::piecewise_construct, std::tuple(val), std::tuple(-val));
        EXPECT_TRUE(map.contains(SMALL_LIMIT / 2));
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, emplace_hint_prev_bound_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(map_test, emplace_hint_overshoot_hint_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(1);
    }

    TYPED_TEST_P(map_test, emplace_hint_right_bound_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(map_test, emplace_hint_undershoot_hint_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(-2);
    }

    TYPED_TEST_P(map_test, emplace_hint_begin_hint_test) {
        typename TypeParam::regular_type map;
        map.emplace(std::piecewise_construct, std::tuple(1), std::tuple(1));
        map.emplace_hint(map.begin(), std::piecewise_construct, std::tuple(0), std::tuple(0));
        EXPECT_EQ(0, map[0]);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, emplace_hint_end_hint_test) {
        typename TypeParam::regular_type map;
        map.emplace(std::piecewise_construct, std::tuple(0), std::tuple(0));
        map.emplace_hint(map.end(), std::piecewise_construct, std::tuple(1), std::tuple(1));
        EXPECT_EQ(1, map[1]);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, try_emplace_lvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(stub1, stub2);
        check_constructor_counts(0, 2, 0);
        EXPECT_EQ(map[stub1], stub2);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, try_emplace_rvalue_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub1(SPECIAL_VALUE);
        constructor_stub stub2(-SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(stub1, std::move(stub2));
        check_constructor_counts(0, 1, 1);
        EXPECT_EQ(map[stub1], stub2);
        EXPECT_TRUE(map.__is_valid());
    }

    TYPED_TEST_P(map_test, try_emplace_id_forward_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(stub, -SPECIAL_VALUE);
        check_constructor_counts(0, 1, 0);
        EXPECT_EQ(map[SPECIAL_VALUE].id, -SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, rvalue_try_emplace_id_forward_test) {
        typename TypeParam::regular_type map;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        map.try_emplace(std::move(stub), -SPECIAL_VALUE);
        check_constructor_counts(0, 0, 1);
        EXPECT_EQ(map[SPECIAL_VALUE].id, -SPECIAL_VALUE);
    }

    TYPED_TEST_P(map_test, try_emplace_return_test) {
        using Map = typename TypeParam::regular_type;
        Map map;
        constructor_stub key(SPECIAL_VALUE);
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<typename Map::iterator, bool> res = map.try_emplace(key, value);
        EXPECT_EQ((*res.first).first, key);
        EXPECT_EQ((*res.first).second, value);
        EXPECT_TRUE(res.second);
        std::pair<typename Map::iterator, bool> res2 = map.try_emplace(key, value);
        EXPECT_EQ((*res2.first).first, key);
        EXPECT_EQ((*res2.first).second, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(map.size(), 1);
    }

    TYPED_TEST_P(map_test, erase_by_key_basic_test) {
        typename TypeParam::regular_type map;
        map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE));
        mark_constructor_counts();
        map.erase(SPECIAL_VALUE);
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(map.find(SPECIAL_VALUE), map.cend());
    }

    TYPED_TEST_P(map_test, erase_by_key_return_value_test) {
        typename TypeParam::regular_type map;
        map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE));
        EXPECT_EQ(map.erase(SPECIAL_VALUE), 1);
        EXPECT_EQ(map.erase(SPECIAL_VALUE), 0);
    }

    TYPED_TEST_P(map_test, erase_by_key_intermediate_test) {
        erase_by_key<typename TypeParam::regular_type>(this -> tracker, SMALL_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_by_key_stress_test) {
        erase_by_key<typename TypeParam::regular_type>(this -> tracker, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_by_iterator_basic_test) {
        using Map = typename TypeParam::regular_type;
        Map map;
        typename Map::iterator it = map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE)).first;
        mark_constructor_counts();
        map.erase(it);
        check_constructor_counts();
        EXPECT_TRUE(map.__is_valid());
        EXPECT_EQ(map.find(SPECIAL_VALUE), map.cend());
    }

    TYPED_TEST_P(map_test, erase_by_iterator_return_value_test) {
        using Map = typename TypeParam::regular_type;
        Map map;
        typename Map::iterator it1 = map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE), std::forward_as_tuple(-SPECIAL_VALUE)).first;
        typename Map::iterator it2 = map.emplace(std::piecewise_construct, std::forward_as_tuple(SPECIAL_VALUE + 1), std::forward_as_tuple(-SPECIAL_VALUE - 1)).first;
        EXPECT_EQ(map.erase(it1), it2);
        EXPECT_EQ(map.erase(it2), map.end());
    }

    TYPED_TEST_P(map_test, erase_by_iterator_intermediate_test) {
        erase_by_iterator<typename TypeParam::regular_type>(this -> tracker, SMALL_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_by_iterator_stress_test) {
        erase_by_iterator<typename TypeParam::regular_type>(this -> tracker, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(map_test, erase_range_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        auto stub_pairs = get_random_stub_pair_vector(stubs);
        typename TypeParam::regular_type map(stub_pairs.cbegin(), stub_pairs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        std::size_t trisect1 = stubs.size() / 3;
        std::size_t trisect2 = stubs.size() - stubs.size() / 3;
        auto it1 = map.find(stubs[trisect1]);
        auto it2 = map.find(stubs[trisect2]);
        EXPECT_EQ(map.erase(it1, it2), it2);
        EXPECT_EQ(map.size(), stubs.size() - (trisect2 - trisect1));
        for (std::size_t i = trisect1; i < trisect2; ++i) {
            EXPECT_EQ(map.find(stubs[i]), map.end());
        }
        map.erase(it2, map.end());
        EXPECT_EQ(map.size(), trisect1);
        for (std::size_t i = trisect2; i < stubs.size(); ++i) {
            EXPECT_EQ(map.find(stubs[i]), map.end());
        }
        map.erase(map.begin(), map.end());
        EXPECT_TRUE(map.empty());
    }

    TYPED_TEST_P(map_test, swap_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs1 = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1(stub_pairs1.cbegin(), stub_pairs1.cend());
        auto stub_pairs2 = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map2(stub_pairs2.cbegin(), stub_pairs2.cend());
        std::swap(map1, map2);
        EXPECT_TRUE(map1.__is_valid());
        EXPECT_TRUE(map2.__is_valid());
        equivalence_test(map2, stub_pairs1);
        equivalence_test(map1, stub_pairs2);
    }

    TYPED_TEST_P(map_test, contains_test) {
        typename TypeParam::regular_type map;
        map[constructor_stub(SPECIAL_VALUE)] = constructor_stub(-SPECIAL_VALUE);
        mark_constructor_counts();
        EXPECT_TRUE(map.contains(SPECIAL_VALUE));
        EXPECT_FALSE(map.contains(-SPECIAL_VALUE));
        check_constructor_counts();
    }

    // Upper bound and lower bound tests
    TYPED_TEST_P(map_test, upper_bound_test) {
        upper_bound_test_template<typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(map_test, upper_bound_const_test) {
        upper_bound_test_template<const typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(map_test, lower_bound_test) {
        lower_bound_test_template<typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(map_test, lower_bound_const_test) {
        lower_bound_test_template<const typename TypeParam::regular_type>();
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
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[1] = -1;
        Map map2;
        map2[2] = -2;
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_basic_parallel_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[1] = -1;
        Map map2;
        map2[2] = -2;
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
    }

    TYPED_TEST_P(map_test, union_of_conflict_basic_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[1] = -1;
        Map map2;
        map2[1] = -2;
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_conflict_basic_parallel_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[1] = -1;
        Map map2;
        map2[1] = -2;
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
    }

    TYPED_TEST_P(map_test, union_of_empty_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        Map map2;
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
        union_of_test_template(this -> tracker, this -> executor, map2, map1, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_intermediate_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        Map map1(stub_pairs.begin(), stub_pairs.begin() + SMALL_LIMIT / 2);
        Map map2(stub_pairs.begin() + SMALL_LIMIT / 2, stub_pairs.end());
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_intermediate_parallel_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        Map map1(stub_pairs.begin(), stub_pairs.begin() + SMALL_LIMIT / 2);
        Map map2(stub_pairs.begin() + SMALL_LIMIT / 2, stub_pairs.end());
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
    }
    
    TYPED_TEST_P(map_test, union_of_conflict_intermediate_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin(), stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.end());
        union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, union_of_stress_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1;
        Map map2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 2; ++i) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, union_of_stress_parallel_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1;
        Map map2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 2; ++i) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(map_test, union_of_conflict_stress_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; ++i) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, union_of_conflict_stress_parallel_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; ++i) {
            map1.insert(stub_pairs[i]);
            unsigned reverse_index = stub_pairs.size() - 1 - i;
            map2.insert(stub_pairs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(map_test, intersection_of_basic_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        map1[1] = -1;
        Map map2;
        map2[0] = 2;
        map2[3] = -3;
        intersection_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, intersection_of_basic_parallel_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        map1[1] = -1;
        Map map2;
        map2[0] = 2;
        map2[3] = -3;
        intersection_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
    }

    TYPED_TEST_P(map_test, intersection_of_empty_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        Map map2;
        map2[1] = -1;
        intersection_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
        intersection_of_test_template(this -> tracker, this -> executor, map2, map1, stub_pair_resolver());
    }

    TYPED_TEST_P(map_test, intersection_of_intermediate_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; ++i) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, intersection_of_stress_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver());
            }
        }
    }

    TYPED_TEST_P(map_test, intersection_of_stress_parallel_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, map1, map2, stub_pair_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(map_test, difference_of_basic_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        map1[1] = -1;
        Map map2;
        map2[0] = 2;
        map1[2] = -2;
        difference_of_test_template(this -> executor, map1, map2);
    }

    TYPED_TEST_P(map_test, difference_of_basic_parallel_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        map1[1] = -1;
        Map map2;
        map2[0] = 2;
        map1[2] = -2;
        difference_of_test_template(this -> executor, map1, map2, true);
    }

    TYPED_TEST_P(map_test, difference_of_empty_test) {
        using Map = typename TypeParam::regular_type;
        Map map1;
        map1[0] = 0;
        Map map2;
        map2[0] = -1;
        difference_of_test_template(this -> executor, map1, map2);
        difference_of_test_template(this -> executor, map2, map1);
    }

    TYPED_TEST_P(map_test, difference_of_intermediate_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(SMALL_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stub_pairs.size() / 3; ++i) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> executor, map1, map2);
            }
        }
    }

    TYPED_TEST_P(map_test, difference_of_stress_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> executor, map1, map2);
            }
        }
    }

    TYPED_TEST_P(map_test, difference_of_stress_parallel_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        unsigned share_start = stub_pairs.size() / 3;
        unsigned share_end = stub_pairs.size() - stub_pairs.size() / 3;
        Map map1(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        Map map2(stub_pairs.begin() + share_start, stub_pairs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            map1.insert(stub_pairs[i]);
            map2.insert(stub_pairs[stub_pairs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> executor, map1, map2, true);
            }
        }
    }

    // Mixed stress tests
    TYPED_TEST_P(map_test, mixed_stress_test) {
        typename TypeParam::regular_type map;
        int SRTESS_LIMIT = 20000;

        std::map<constructor_stub, constructor_stub, constructor_stub_comparator> stub_map;
        for (int i = 0; i < SRTESS_LIMIT; ++i) {
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

    TYPED_TEST_P(map_test, copy_only_constructor_test) {
        using Map = typename TypeParam::copy_only_type;
        std::vector<std::pair<const copy_only_constructor_stub, copy_only_constructor_stub> > stub_pairs;
        Map map1;
        Map map2(stub_pairs.begin(), stub_pairs.end());
        Map map3(std::make_move_iterator(stub_pairs.begin()), std::make_move_iterator(stub_pairs.end()));
        Map map4(map2);
        Map map5(std::move(map2));
        std::vector<std::pair<copy_only_constructor_stub, copy_only_constructor_stub> > stub_pairs2;
        Map map6(stub_pairs2.begin(), stub_pairs2.end());
        Map map7(std::make_move_iterator(stub_pairs2.begin()), std::make_move_iterator(stub_pairs2.end()));
    }

    TYPED_TEST_P(map_test, copy_only_operation_test) {
        using Map = typename TypeParam::copy_only_type;
        std::vector<std::pair<const copy_only_constructor_stub, copy_only_constructor_stub> > stub_pairs(SMALL_LIMIT);
        Map map1;
        map1.insert(stub_pairs.begin(), stub_pairs.end());
        map1.insert(stub_pairs[0]);
        map1.insert(std::move(stub_pairs[0]));
        map1.emplace(copy_only_constructor_stub(SPECIAL_VALUE), copy_only_constructor_stub(SPECIAL_VALUE2));
        map1[stub_pairs[0].first] = stub_pairs[0].first;
        map1[stub_pairs[0].first] = std::move(stub_pairs[0].first);
        map1.find(stub_pairs[0].first);
        map1.lower_bound(stub_pairs[0].first);
        map1.upper_bound(stub_pairs[0].first);
        for (auto& p : map1) {
            p.second = p.first;
        }
        map1.erase(stub_pairs[0].first);
        map1.erase(map1.begin());
        map1.erase(map1.begin(), map1.end());
        Map map2, map3;
        Map map = union_of(Map(map2), Map(map3), this -> executor, chooser<typename Map::value_type>());
        map = intersection_of(Map(map2), Map(map3), this -> executor, chooser<typename Map::value_type>());
        map = difference_of(Map(map2), Map(map3), this -> executor);
        map2.swap(map3);
        EXPECT_FALSE(map1.contains(stub_pairs[0].first));
    }

    TYPED_TEST_P(map_test, move_only_constructor_test) {
        using Map = typename TypeParam::move_only_type;
        std::vector<std::pair<const constructor_stub, move_only_constructor_stub> > stub_pairs;
        Map map1;
        Map map2(std::make_move_iterator(stub_pairs.begin()), std::make_move_iterator(stub_pairs.end()));
        Map map3(std::move(map2));
        std::vector<std::pair<constructor_stub, move_only_constructor_stub> > stub_pairs2;
        Map map4(std::make_move_iterator(stub_pairs2.begin()), std::make_move_iterator(stub_pairs2.end()));
    }

    TYPED_TEST_P(map_test, move_only_operation_test) {
        using Map = typename TypeParam::move_only_type;
        std::vector<std::pair<const constructor_stub, move_only_constructor_stub> > stub_pairs(SMALL_LIMIT);
        Map map1;
        map1.insert(std::make_move_iterator(stub_pairs.begin()), std::make_move_iterator(stub_pairs.end()));
        map1.insert(std::move(stub_pairs[0]));
        map1.emplace(constructor_stub(SPECIAL_VALUE), move_only_constructor_stub(SPECIAL_VALUE2));
        map1[stub_pairs[0].first] = std::move(stub_pairs[0].second);
        map1.find(stub_pairs[0].first);
        map1.lower_bound(stub_pairs[0].first);
        map1.upper_bound(stub_pairs[0].first);
        for (auto& p : map1) {
            p.second.id = p.first.id;
        }
        map1.erase(stub_pairs[0].first);
        map1.erase(map1.begin());
        map1.erase(map1.begin(), map1.end());
        Map map2 = union_of(Map(), Map(), this -> executor, chooser<typename Map::value_type>());
        map2 = intersection_of(Map(), Map(), this -> executor, chooser<typename Map::value_type>());
        map2 = difference_of(Map(), Map(), this -> executor);
        map2.swap(map1);
        EXPECT_FALSE(map1.contains(stub_pairs[0].first));
    }

    TYPED_TEST_P(map_test, equality_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1(stub_pairs.cbegin(), stub_pairs.cend());
        Map map2(stub_pairs.cbegin(), stub_pairs.cend());
        EXPECT_EQ(map1, map2);
    }

    TYPED_TEST_P(map_test, inequality_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1(stub_pairs.cbegin(), stub_pairs.cend());
        Map map2(stub_pairs.cbegin(), stub_pairs.cend() - 1);
        EXPECT_NE(map1, map2);
    }

    TYPED_TEST_P(map_test, three_way_comparison_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1(stub_pairs.cbegin(), stub_pairs.cend());
        stub_pairs.back().second.id++;
        Map map2(stub_pairs.cbegin(), stub_pairs.cend());
        EXPECT_LE(map1, map2);
        EXPECT_LT(map1, map2);
        EXPECT_GT(map2, map1);
        EXPECT_GE(map2, map1);
    }

    TYPED_TEST_P(map_test, three_way_comparison_length_test) {
        using Map = typename TypeParam::regular_type;
        auto stub_pairs = get_random_stub_pair_vector(MEDIUM_LIMIT);
        Map map1(stub_pairs.cbegin(), stub_pairs.cend());
        map1.erase(std::prev(map1.end()));
        Map map2(stub_pairs.cbegin(), stub_pairs.cend());
        EXPECT_LE(map1, map2);
        EXPECT_LT(map1, map2);
        EXPECT_GT(map2, map1);
        EXPECT_GE(map2, map1);
    }

    REGISTER_TYPED_TEST_SUITE_P(map_test,
        trait_test,
        default_constructor_test,
        comp_allocator_constructor_test,
        comp_allocator_constructor_optional_test,
        allocator_constructor_test,
        comp_allocator_range_constructor_test,
        comp_allocator_range_constructor_optional_allocator_test,
        comp_allocator_range_constructor_both_optional_test,
        allocator_range_constructor_test,
        copy_constructor_test,
        allocator_copy_constructor_test,
        move_constructor_test,
        allocator_move_constructor_test,
        assignment_operator_test,
        move_assignment_operator_test, 
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
        empty_test,
        size_test,
        clear_test,
        const_ref_insert_find_basic_test,
        rvalue_insert_find_basic_test,
        template_lvalue_insert_find_basic_test,
        template_rvalue_insert_find_basic_test,
        insert_return_value_test,
        insert_find_intermediate_test,
        insert_find_stress_test,
        insert_hint_equal_hint_test,
        insert_hint_equal_next_hint_test,
        insert_hint_equal_prev_hint_test,
        insert_hint_overshoot_hint_test,
        insert_hint_prev_bound_test,
        insert_hint_right_bound_test,
        insert_hint_undershoot_hint_test,
        insert_hint_begin_hint_test,
        insert_hint_end_hint_test,
        insert_range_test,
        emplace_lvalue_test,
        emplace_rvalue_test,
        emplace_id_forward_test,
        emplace_return_test,
        emplace_hint_begin_hint_test,
        emplace_hint_end_hint_test,
        emplace_hint_equal_hint_test,
        emplace_hint_equal_next_hint_test,
        emplace_hint_equal_prev_hint_test,
        emplace_hint_overshoot_hint_test,
        emplace_hint_prev_bound_test,
        emplace_hint_right_bound_test,
        emplace_hint_undershoot_hint_test,
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
        upper_bound_test,
        upper_bound_const_test,
        lower_bound_test,
        lower_bound_const_test,
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
        mixed_stress_test,
        copy_only_constructor_test,
        copy_only_operation_test,
        move_only_constructor_test,
        move_only_operation_test,
        equality_test,
        inequality_test,
        three_way_comparison_test,
        three_way_comparison_length_test
    );
}
