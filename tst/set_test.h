#pragma once
#include <concepts>
#include "gtest/gtest.h"
#include "utility/constructor_stub.h"
#include "utility/common.h"
#include "utility/tracking_allocator.h"
#include "utility/copy_only_constructor_stub.h"
#include "utility/move_only_constructor_stub.h"
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
    class set_test : public testing::Test {
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

    template<typename Set>
    void equivalence_test(const Set& set, std::vector<typename Set::value_type>& stubs) {
        for (auto& stub : stubs) {
            EXPECT_TRUE(set.contains(stub));
            EXPECT_EQ(stub, *set.find(stub));
        }
        EXPECT_EQ(set.size(), stubs.size());
    }

    template<typename Set>
    void begin_end_test_template() {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set(stubs.begin(), stubs.end());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        int i = 0;
        mark_constructor_counts();
        for (auto& stub : set) {
            EXPECT_EQ(stub, stubs[i]);
            EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(stub)>>,
                std::is_const_v<Set>);
            ++i;
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    template<typename Set>
    void rbegin_rend_test_template(construction_destruction_tracker& tracker) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        tracker.mark();
        Set set(stubs.begin(), stubs.end());
        tracker.check_marked();
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        mark_constructor_counts();
        for (auto it = set.rbegin(); it != set.rend(); ++it, ++i) {
            EXPECT_EQ(*it, stubs[i]);
            EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(*it)>>,
                std::is_const_v<Set>);
        }
        EXPECT_EQ(i, MEDIUM_LIMIT);
        check_constructor_counts();
    }
    
    template<typename Set>
    void insert_find_test(construction_destruction_tracker& tracker, std::size_t size) {
        Set set;
        auto stubs = get_random_stub_vector(size);
        int i = 0;
        unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
        for (auto& stub : stubs) {
            mark_constructor_counts();
            tracker.mark();
            set.insert(stub);
            tracker.check_marked();
            check_constructor_counts(0, 1, 0);
            EXPECT_EQ(set.size(), i + 1);
            if (i % skip == 0) {
                for (int j = 0; j <= i; ++j) {
                    EXPECT_TRUE(set.contains(stubs[j]));
                }
                EXPECT_TRUE(set.__is_valid());
            }
            mark_constructor_counts();
            set.insert(stub);
            check_constructor_counts(0, 0, 0);
            EXPECT_EQ(set.size(), i + 1);
            if (i % skip == 0) {
                for (int j = 0; j <= i; ++j) {
                    EXPECT_TRUE(set.contains(stubs[j]));
                }
                EXPECT_TRUE(set.__is_valid());
            }
            ++i;
        }
    }

    template<typename Set>
    void erase_by_key(construction_destruction_tracker& tracker, std::size_t size) {
        auto stubs = get_random_stub_vector(size);
        Set set(stubs.cbegin(), stubs.cend());
        std::size_t curr_size = size;
        unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
        for (auto& stub : stubs) {
            EXPECT_NE(set.find(stub), set.end());
            mark_constructor_counts();
            tracker.mark();
            set.erase(stub);
            tracker.check_marked();
            check_constructor_counts();
            --curr_size;
            EXPECT_EQ(set.find(stub), set.cend());
            if (curr_size % skip == 0) {
                EXPECT_TRUE(set.__is_valid());
            }
        }
        EXPECT_TRUE(set.empty());
    }

    template<typename Set>
    void erase_by_iterator(construction_destruction_tracker& tracker, std::size_t size) {
        auto stubs = get_random_stub_vector(size);
        Set set(stubs.cbegin(), stubs.cend());
        std::size_t curr_size = size;
        typename Set::iterator it = set.begin();
        unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
        while (!set.empty()) {
            typename Set::value_type stub = *it;
            EXPECT_NE(set.find(stub), set.end());
            EXPECT_EQ(1, set.count(stub));
            mark_constructor_counts();
            tracker.mark();
            it = set.erase(it);
            tracker.check_marked();
            --curr_size;
            check_constructor_counts();
            EXPECT_EQ(set.find(stub), set.cend());
            EXPECT_EQ(0, set.count(stub));
            EXPECT_EQ(set.size(), curr_size);
            if (curr_size % skip == 0) {
                EXPECT_TRUE(set.__is_valid());
            }
        }
        EXPECT_TRUE(set.empty());
        EXPECT_EQ(it, set.end());
    }

    template<typename Set>
    void upper_bound_test_template() {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        // Create gap between all numbers
        for (auto& stub : stubs) {
            stub.id *= 2;
        }
        using raw_type = std::remove_const_t<Set>;
        raw_type src(stubs.cbegin(), stubs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        Set& set = src;
        // With existing element
        for (unsigned i = 0; i + 1 < stubs.size(); ++i) {
            EXPECT_EQ(*set.upper_bound(stubs[i]), stubs[i + 1]);
        }
        // With absent element
        for (unsigned i = 0; i < stubs.size() - 1; ++i) {
            if (stubs[i + 1].id - stubs[i].id >= 2) {
                int num = (stubs[i + 1].id + stubs[i].id) / 2;
                auto& stub = *set.upper_bound(constructor_stub(num));
                EXPECT_EQ(stub.id, stubs[i + 1].id);
                EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(stub)>>,
                        std::is_const_v<Set>);
            }
        }
        EXPECT_EQ(set.upper_bound(stubs.back().id + 1), set.end());
    }

    template <typename Set>
    void lower_bound_test_template() {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        // Create gap between all numbers
        for (auto& stub : stubs) {
            stub.id *= 2;
        }
        using raw_type = std::remove_const_t<Set>;
        raw_type src(stubs.cbegin(), stubs.cend());
        Set& set = src;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        // With existing element
        for (unsigned i = 0; i < stubs.size(); ++i) {
            EXPECT_EQ(*set.lower_bound(stubs[i]), stubs[i]);
        }
        // With absent element
        for (unsigned i = 1; i < stubs.size(); ++i) {
            if (stubs[i].id - stubs[i - 1].id >= 2) {
                int num = (stubs[i].id + stubs[i - 1].id) / 2;
                auto& stub = *set.lower_bound(constructor_stub(num));
                EXPECT_EQ(stub.id, stubs[i].id);
                EXPECT_EQ(std::is_const_v<std::remove_reference_t<decltype(stub)>>,
                        std::is_const_v<Set>);
            }
        }
        EXPECT_EQ(set.lower_bound(stubs.back().id + 1), set.end());
    }

    template<typename Set, typename Resolver>
    void union_of_test_template(construction_destruction_tracker& tracker, thread_pool_executor& executor, 
                                const Set& set1, const Set& set2, Resolver resolver, bool is_parallel=false) {
        Set set_copy1(set1);
        Set set_copy2(set2);
        Set set;
        mark_constructor_counts();
        tracker.mark();
        if (is_parallel) {
            set = union_of<Resolver>(std::move(set_copy1), std::move(set_copy2), executor, resolver);
        } else {
            set = union_of<Resolver>(std::move(set_copy1), std::move(set_copy2), resolver);
        }
        tracker.check_marked();
        check_constructor_counts();
        EXPECT_TRUE(set.__is_valid());
        // No element is missing
        for (const auto& stub : set1) {
            EXPECT_NE(set.find(stub), set.end());
        }
        for (const auto& stub : set2) {
            EXPECT_NE(set.find(stub), set.end());
        }
        // No extraneous elements
        for (const auto& stub : set) {
            EXPECT_TRUE(set1.find(stub) != set1.end() || set2.find(stub) != set2.end());
        }
        // Resolution is correct
        for (const auto& stub1 : set1) {
            if (set2.find(stub1) != set2.end()) {
                const auto& stub2 = *set2.find(stub1);
                const auto& stub = *set.find(stub1);
                if (resolver(stub1, stub2)) {
                    EXPECT_EQ(stub, stub1);
                } else {
                    EXPECT_EQ(stub, stub2);
                }
            }
        }
    }

    template<typename Set, typename Resolver>
    void intersection_of_test_template(construction_destruction_tracker& tracker, thread_pool_executor& executor,
                                       const Set& set1, const Set& set2, Resolver resolver, bool is_parallel=false) {
        Set set_copy1(set1);
        Set set_copy2(set2);
        Set set;
        mark_constructor_counts();
        tracker.mark();
        if (is_parallel) {
            set = intersection_of<Resolver>(std::move(set_copy1), std::move(set_copy2), executor, resolver);
        } else {
            set = intersection_of<Resolver>(std::move(set_copy1), std::move(set_copy2), resolver);
        }
        tracker.check_marked();
        check_constructor_counts();
        EXPECT_TRUE(set.__is_valid());
        // No extraneous elements
        for (const auto& stub : set) {
            EXPECT_TRUE(set1.find(stub) != set1.end() && set2.find(stub) != set2.end());
        }
        // Resolution is correct
        for (const auto& stub1 : set1) {
            if (set2.find(stub1) != set2.end()) {
                const auto& stub2 = *set2.find(stub1);
                EXPECT_NE(set.find(stub1), set.end());
                const auto& stub = *set.find(stub1);
                if (resolver(stub1, stub2)) {
                    EXPECT_EQ(stub, stub1);
                } else {
                    EXPECT_EQ(stub, stub2);
                }
            }
        }
    }

    template<typename Set>
    void difference_of_test_template(construction_destruction_tracker& tracker, thread_pool_executor& executor, const Set& set1, const Set& set2, bool is_parallel=false) {
        Set set_copy1(set1);
        Set set_copy2(set2);
        Set set;
        mark_constructor_counts();
        tracker.mark();
        if (is_parallel) {
            set = difference_of(std::move(set_copy1), std::move(set_copy2), executor);
        } else {
            set = difference_of(std::move(set_copy1), std::move(set_copy2));
        }
        tracker.check_marked();
        check_constructor_counts();
        EXPECT_TRUE(set.__is_valid());
        // No extraneous elements
        for (const auto& stub : set) {
            EXPECT_TRUE(set1.find(stub) != set1.end() && set2.find(stub) == set2.end());
        }
        for (const auto& stub : set1) {
            if (set2.find(stub) == set2.end()) {
                EXPECT_NE(set.find(stub), set.end());
            }
        }
    }



    TYPED_TEST_SUITE_P(set_test);

    TYPED_TEST_P(set_test, trait_test) {
        using Set = typename TypeParam::regular_type;
        using const_node_type = constify<constructor_stub, typename Set::node_type>;
        bool correct = std::is_same_v<typename Set::key_type, constructor_stub>
            && std::is_same_v<typename Set::value_type, constructor_stub>
            && std::is_same_v<typename Set::size_type, std::size_t>
            && std::is_same_v<typename Set::difference_type, std::ptrdiff_t>
            && std::is_same_v<typename Set::value_compare, constructor_stub_comparator>
            && std::is_same_v<typename Set::allocator_type, tracking_allocator<constructor_stub> >
            && std::is_same_v<typename Set::reference, constructor_stub&>
            && std::is_same_v<typename Set::const_reference, const constructor_stub&>
            && std::is_same_v<typename Set::pointer, constructor_stub*>
            && std::is_same_v<typename Set::const_pointer, const constructor_stub*>
            && std::is_same_v<typename Set::iterator, binary_tree_iterator<typename Set::node_type> >
            && std::is_same_v<typename Set::const_iterator, binary_tree_iterator<const_node_type> >
            && std::is_same_v<typename Set::reverse_iterator, std::reverse_iterator<binary_tree_iterator<typename Set::node_type>> >
            && std::is_same_v<typename Set::const_reverse_iterator, std::reverse_iterator<binary_tree_iterator<const_node_type>> >;
        EXPECT_TRUE(correct);
    }
    
    template <typename Set>
    void default_constructor_test_helper(Set& set) {
        EXPECT_TRUE(set.__is_valid());
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(allocated<typename Set::node_type>, 1);
    }

    TYPED_TEST_P(set_test, default_constructor_test) {
        typename TypeParam::regular_type set;
        default_constructor_test_helper(set);
    }

    TYPED_TEST_P(set_test, allocator_constructor_test) {
        typename TypeParam::regular_type set(this -> allocator);
        default_constructor_test_helper(set);
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    template <typename Set, typename Comp>
    void comp_allocator_constructor_test_helper(Set& set, Comp& comp) {
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(allocated<typename Set::node_type>, 1);
        EXPECT_EQ(comp, set.value_comp());
    }

    TYPED_TEST_P(set_test, comp_allocator_constructor_test) {
        constructor_stub_comparator comp(true);
        typename TypeParam::regular_type set(comp, this -> allocator);
        comp_allocator_constructor_test_helper(set, comp);
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    TYPED_TEST_P(set_test, comp_allocator_constructor_optional_test) {
        constructor_stub_comparator comp(true);
        typename TypeParam::regular_type set(comp);
        comp_allocator_constructor_test_helper(set, comp);
    }

    template <typename Set, typename Stubs>
    void comp_allocator_range_constructor_test_helper(Set& set, Stubs& stubs) {
        equivalence_test(set, stubs);
        EXPECT_EQ(allocated<typename Set::node_type>, stubs.size() + 1);
    }

    TYPED_TEST_P(set_test, comp_allocator_range_constructor_test) {
        constructor_stub_comparator comp;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        typename TypeParam::regular_type set(stubs.begin(), stubs.end(), comp, this -> allocator);
        comp_allocator_range_constructor_test_helper(set, stubs);
        EXPECT_EQ(comp, set.value_comp());
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    TYPED_TEST_P(set_test, comp_allocator_range_constructor_optional_allocator_test) {
        constructor_stub_comparator comp;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        typename TypeParam::regular_type set(stubs.begin(), stubs.end(), comp);
        comp_allocator_range_constructor_test_helper(set, stubs);
        EXPECT_EQ(comp, set.value_comp());
    }

    TYPED_TEST_P(set_test, comp_allocator_range_constructor_both_optional_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        typename TypeParam::regular_type set(stubs.begin(), stubs.end());
        comp_allocator_range_constructor_test_helper(set, stubs);
    }

    TYPED_TEST_P(set_test, allocator_range_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        typename TypeParam::regular_type set(stubs.begin(), stubs.end(), this -> allocator);
        comp_allocator_range_constructor_test_helper(set, stubs);
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    TYPED_TEST_P(set_test, copy_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        using Set = typename TypeParam::regular_type;
        constructor_stub_comparator comp(true);
        Set set(comp);
        set.insert(stubs.begin(), stubs.end());
        EXPECT_TRUE(set.__is_valid());
        std::size_t alloc_count = allocated<typename Set::node_type>;
        Set set_copy(set);
        EXPECT_EQ(alloc_count + SMALL_LIMIT + 1, allocated<typename Set::node_type>);
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        equivalence_test(set_copy, stubs);
        EXPECT_EQ(set.get_allocator(), set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, allocator_copy_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Set = typename TypeParam::regular_type;
        Set set(comp);
        set.insert(stubs.begin(), stubs.end());
        std::size_t alloc_count = allocated<typename Set::node_type>;
        Set set_copy(set, this -> allocator);
        EXPECT_EQ(alloc_count + SMALL_LIMIT + 1, allocated<typename Set::node_type>);
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        equivalence_test(set_copy, stubs);
        EXPECT_EQ(this -> allocator, set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, move_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Set = typename TypeParam::regular_type;
        Set set(comp, this -> allocator);
        set.insert(stubs.begin(), stubs.end());
        mark_constructor_counts();
        std::size_t alloc_count = allocated<typename Set::node_type>;
        Set set_copy(std::move(set));
        EXPECT_EQ(alloc_count, allocated<typename Set::node_type>);
        check_constructor_counts();
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        equivalence_test(set_copy, stubs);
        EXPECT_EQ(this -> allocator, set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, allocator_move_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Set = typename TypeParam::regular_type;
        Set set(comp);
        set.insert(stubs.begin(), stubs.end());
        mark_constructor_counts();
        std::size_t alloc_count = allocated<typename Set::node_type>;
        Set set_copy(std::move(set), this -> allocator);
        EXPECT_EQ(alloc_count, allocated<typename Set::node_type>);
        check_constructor_counts();
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        equivalence_test(set_copy, stubs);
        EXPECT_EQ(this -> allocator, set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, assignment_operator_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Set = typename TypeParam::regular_type;
        Set set1(comp);
        set1.insert(stubs.begin(), stubs.end());
        Set set2;
        set2 = set1;
        EXPECT_TRUE(set2.__is_valid());
        equivalence_test(set2, stubs);
    }

    TYPED_TEST_P(set_test, move_assignment_operator_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        using Set = typename TypeParam::regular_type;
        Set set1(comp);
        set1.insert(stubs.begin(), stubs.end());
        Set set2;
        mark_constructor_counts();
        set2 = std::move(set1);
        check_constructor_counts();
        EXPECT_TRUE(set2.__is_valid());
        equivalence_test(set2, stubs);
    }

    // Iterator tests
    TYPED_TEST_P(set_test, begin_end_test) {
        begin_end_test_template<typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(set_test, const_begin_end_test) {
        begin_end_test_template<const typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(set_test, cbegin_cend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        const typename TypeParam::regular_type set(stubs.begin(), stubs.end());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        mark_constructor_counts();
        for (auto it = set.cbegin(); it != set.cend(); ++it, ++i) {
            EXPECT_EQ(*it, stubs[i]);
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, rbegin_rend_test) {
        rbegin_rend_test_template<typename TypeParam::regular_type>(this -> tracker);
    }

    TYPED_TEST_P(set_test, const_rbegin_rend_test) {
        rbegin_rend_test_template<const typename TypeParam::regular_type>(this -> tracker);
    }

    TYPED_TEST_P(set_test, crbegin_crend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        const typename TypeParam::regular_type set(stubs.begin(), stubs.end());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        mark_constructor_counts();
        for (auto it = set.crbegin(); it != set.crend(); ++it, ++i) {
            EXPECT_EQ(*it, stubs[i]);
        }
        check_constructor_counts();
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, empty_test) {
        typename TypeParam::regular_type set;
        EXPECT_TRUE(set.empty());
        set.emplace(SPECIAL_VALUE);
        EXPECT_FALSE(set.empty());
    }

    TYPED_TEST_P(set_test, size_test) {
        typename TypeParam::regular_type set;
        EXPECT_EQ(set.size(), 0);
        set.emplace(SPECIAL_VALUE);
        EXPECT_EQ(set.size(), 1);
    }

    TYPED_TEST_P(set_test, clear_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        typename TypeParam::regular_type set(stubs.begin(), stubs.end());
        EXPECT_TRUE(set.__is_valid());
        set.clear();
        EXPECT_TRUE(set.empty());
        EXPECT_TRUE(set.__is_valid());
    }
    
    // Core methods (find and insert) tests
    TYPED_TEST_P(set_test, const_ref_insert_find_basic_test) {
        using Set = typename TypeParam::regular_type;
        constructor_stub stub(SPECIAL_VALUE);
        Set set;
        EXPECT_EQ(set.find(stub), set.cend());
        const typename Set::value_type& const_ref = stub;
        mark_constructor_counts();
        set.insert(const_ref);
        check_constructor_counts(0, 1, 0);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_TRUE(set.contains(stub));
    }

    TYPED_TEST_P(set_test, rvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        typename TypeParam::regular_type set;
        EXPECT_EQ(set.find(stub), set.cend());
        mark_constructor_counts();
        set.insert(std::move(stub));
        check_constructor_counts(0, 0, 1);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_EQ(set.find(stub) -> id, SPECIAL_VALUE);
        stub = constructor_stub(SPECIAL_VALUE);
        mark_constructor_counts();
        set.insert(std::move(stub));
        check_constructor_counts();
    }

    TYPED_TEST_P(set_test, insert_return_value_test) {
        using Set = typename TypeParam::regular_type;
        Set set;
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<typename Set::iterator, bool> res = set.insert(value);
        EXPECT_EQ(*(res.first), value);
        EXPECT_TRUE(res.second);
        std::pair<typename Set::iterator, bool> res2 = set.insert(value);
        EXPECT_EQ(*(res2.first), value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(set.size(), 1);
    }


    TYPED_TEST_P(set_test, insert_find_intermediate_test) {
        insert_find_test<typename TypeParam::regular_type>(this -> tracker, SMALL_LIMIT);
    }

    TYPED_TEST_P(set_test, insert_find_stress_test) {
        insert_find_test<typename TypeParam::regular_type>(this -> tracker, MEDIUM_LIMIT);
    }

    template<typename Set>
    void insert_hint_equal_test_helper(int hint_delta) {
        Set set;
        typename Set::value_type element(SMALL_LIMIT / 2);
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            set.emplace(i);
        }
        mark_constructor_counts();
        auto it = set.insert(std::next(set.begin(), SMALL_LIMIT / 2 + hint_delta), element);
        check_constructor_counts();
        EXPECT_EQ(it, set.find(SMALL_LIMIT / 2));
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, insert_hint_equal_hint_test) {
        insert_hint_equal_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(set_test, insert_hint_equal_prev_hint_test) {
        insert_hint_equal_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(set_test, insert_hint_equal_next_hint_test) {
        insert_hint_equal_test_helper<typename TypeParam::regular_type>(1);
    }

    template<typename Set>
    void insert_hint_success_test_helper(int hint_delta) {
        Set set;
        typename Set::value_type element(SMALL_LIMIT / 2);
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            set.emplace(i);
        }
        set.erase(SMALL_LIMIT / 2);
        mark_constructor_counts();
        auto it = set.insert(std::next(set.begin(), SMALL_LIMIT / 2 + hint_delta), element);
        EXPECT_TRUE(set.contains(SMALL_LIMIT / 2));
        check_constructor_counts(0, 1, 0);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_EQ(it, set.find(SMALL_LIMIT / 2));
    }

    TYPED_TEST_P(set_test, insert_hint_prev_bound_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(set_test, insert_hint_overshoot_hint_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(1);
    }

    TYPED_TEST_P(set_test, insert_hint_right_bound_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(set_test, insert_hint_undershoot_hint_test) {
        insert_hint_success_test_helper<typename TypeParam::regular_type>(-2);
    }

    TYPED_TEST_P(set_test, insert_hint_begin_hint_test) {
        typename TypeParam::regular_type set;
        set.emplace(1);
        constructor_stub stub(0);
        set.insert(set.begin(), stub);
        EXPECT_TRUE(set.contains(0));
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, insert_hint_end_hint_test) {
        typename TypeParam::regular_type set;
        set.emplace(0);
        constructor_stub stub(1);
        set.insert(set.end(), stub);
        EXPECT_TRUE(set.contains(1));
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, insert_range_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        typename TypeParam::regular_type set(stubs.begin(), stubs.end());
        for (unsigned i = 0; i < stubs.size(); ++i) {
            EXPECT_TRUE(set.contains(stubs[i]));
        }
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, emplace_lvalue_test) {
        typename TypeParam::regular_type set;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        set.emplace(stub);
        check_constructor_counts(0, 1, 0);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_TRUE(set.contains(stub));
    }

    TYPED_TEST_P(set_test, emplace_rvalue_test) {
        typename TypeParam::regular_type set;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        set.emplace(std::move(stub));
        check_constructor_counts(0, 0, 1);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_TRUE(set.contains(stub));
    }

    TYPED_TEST_P(set_test, emplace_id_forward_test) {
        typename TypeParam::regular_type set;
        mark_constructor_counts();
        set.emplace(SPECIAL_VALUE);
        check_constructor_counts();
        EXPECT_TRUE(set.contains(SPECIAL_VALUE));
    }

    TYPED_TEST_P(set_test, emplace_return_test) {
        using Set = typename TypeParam::regular_type;
        Set set;
        constructor_stub value(SPECIAL_VALUE);
        std::pair<typename Set::iterator, bool> res = set.emplace(value);
        EXPECT_EQ(*res.first, value);
        EXPECT_TRUE(res.second);
        std::pair<typename Set::iterator, bool> res2 = set.emplace(value);
        EXPECT_EQ(*res2.first, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(set.size(), 1);
    }

    template<typename Set>
    void emplace_hint_equal_test_helper(int hint_delta) {
        Set set;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            set.emplace(i);
        }
        mark_constructor_counts();
        set.emplace_hint(std::next(set.begin(), SMALL_LIMIT / 2 + hint_delta), SMALL_LIMIT / 2);
        check_constructor_counts();
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, emplace_hint_equal_hint_test) {
        emplace_hint_equal_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(set_test, emplace_hint_equal_prev_hint_test) {
        emplace_hint_equal_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(set_test, emplace_hint_equal_next_hint_test) {
        emplace_hint_equal_test_helper<typename TypeParam::regular_type>(1);
    }

    template<typename Set>
    void emplace_hint_success_test_helper(int hint_delta) {
        Set set;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            set.emplace(i);
        }
        set.erase(SMALL_LIMIT / 2);
        mark_constructor_counts();
        set.emplace_hint(std::next(set.begin(), SMALL_LIMIT / 2 + hint_delta), SMALL_LIMIT / 2);
        EXPECT_TRUE(set.contains(SMALL_LIMIT / 2));
        check_constructor_counts();
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, emplace_hint_prev_bound_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(0);
    }

    TYPED_TEST_P(set_test, emplace_hint_overshoot_hint_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(1);
    }

    TYPED_TEST_P(set_test, emplace_hint_right_bound_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(-1);
    }

    TYPED_TEST_P(set_test, emplace_hint_undershoot_hint_test) {
        emplace_hint_success_test_helper<typename TypeParam::regular_type>(-2);
    }

    TYPED_TEST_P(set_test, emplace_hint_begin_hint_test) {
        typename TypeParam::regular_type set;
        set.emplace(1);
        set.emplace_hint(set.begin(), 0);
        EXPECT_TRUE(set.contains(0));
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, emplace_hint_end_hint_test) {
        typename TypeParam::regular_type set;
        set.emplace(0);
        set.emplace_hint(set.end(), 1);
        EXPECT_TRUE(set.contains(1));
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, erase_by_key_basic_test) {
        typename TypeParam::regular_type set;
        set.emplace(SPECIAL_VALUE);
        mark_constructor_counts();
        set.erase(SPECIAL_VALUE);
        EXPECT_TRUE(set.__is_valid());
        check_constructor_counts();
        EXPECT_EQ(set.find(SPECIAL_VALUE), set.cend());
    }

    TYPED_TEST_P(set_test, erase_by_key_return_value_test) {
        typename TypeParam::regular_type set;
        set.emplace(SPECIAL_VALUE);
        EXPECT_EQ(set.erase(SPECIAL_VALUE), 1);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_EQ(set.erase(SPECIAL_VALUE), 0);
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, erase_by_key_intermediate_test) {
        erase_by_key<typename TypeParam::regular_type>(this -> tracker, SMALL_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_by_key_stress_test) {
        erase_by_key<typename TypeParam::regular_type>(this -> tracker, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_by_iterator_basic_test) {
        using Set = typename TypeParam::regular_type;
        Set set;
        typename Set::iterator it = set.emplace(SPECIAL_VALUE).first;
        mark_constructor_counts();
        set.erase(it);
        check_constructor_counts();
        EXPECT_TRUE(set.__is_valid());
        EXPECT_FALSE(set.contains(SPECIAL_VALUE));
    }

    TYPED_TEST_P(set_test, erase_by_iterator_return_value_test) {
        using Set = typename TypeParam::regular_type;
        Set set;
        typename Set::iterator it1 = set.emplace(SPECIAL_VALUE).first;
        typename Set::iterator it2 = set.emplace(SPECIAL_VALUE + 1).first;
        EXPECT_EQ(set.erase(it1), it2);
        EXPECT_TRUE(set.__is_valid());
        EXPECT_EQ(set.erase(it2), set.end());
    }

    TYPED_TEST_P(set_test, erase_by_iterator_intermediate_test) {
        erase_by_iterator<typename TypeParam::regular_type>(this -> tracker, SMALL_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_by_iterator_stress_test) {
        erase_by_iterator<typename TypeParam::regular_type>(this -> tracker, MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_range_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        typename TypeParam::regular_type set(stubs.cbegin(), stubs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        std::size_t trisect1 = stubs.size() / 3;
        std::size_t trisect2 = stubs.size() - stubs.size() / 3;
        auto it1 = set.find(stubs[trisect1]);
        auto it2 = set.find(stubs[trisect2]);
        EXPECT_EQ(set.erase(it1, it2), it2);
        EXPECT_EQ(set.size(), stubs.size() - (trisect2 - trisect1));
        for (std::size_t i = trisect1; i < trisect2; ++i) {
            EXPECT_EQ(set.find(stubs[i]), set.end());
        }
        set.erase(it2, set.end());
        EXPECT_EQ(set.size(), trisect1);
        for (std::size_t i = trisect2; i < stubs.size(); ++i) {
            EXPECT_EQ(set.find(stubs[i]), set.end());
        }
        set.erase(set.begin(), set.end());
        EXPECT_TRUE(set.empty());
    }

    TYPED_TEST_P(set_test, swap_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs1 = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1(stubs1.cbegin(), stubs1.cend());
        auto stubs2 = get_random_stub_vector(MEDIUM_LIMIT);
        Set set2(stubs2.cbegin(), stubs2.cend());
        std::swap(set1, set2);
        EXPECT_TRUE(set1.__is_valid());
        EXPECT_TRUE(set2.__is_valid());
        equivalence_test(set2, stubs1);
        equivalence_test(set1, stubs2);
    }

    TYPED_TEST_P(set_test, contains_test) {
        typename TypeParam::regular_type set;
        set.emplace(SPECIAL_VALUE);
        mark_constructor_counts();
        EXPECT_TRUE(set.contains(SPECIAL_VALUE));
        EXPECT_FALSE(set.contains(-SPECIAL_VALUE));
        check_constructor_counts();
    }

    // Upper bound and lower bound tests
    TYPED_TEST_P(set_test, upper_bound_test) {
        upper_bound_test_template<typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(set_test, upper_bound_const_test) {
        upper_bound_test_template<const typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(set_test, lower_bound_test) {
        lower_bound_test_template<typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(set_test, lower_bound_const_test) {
        lower_bound_test_template<const typename TypeParam::regular_type>();
    }

    TYPED_TEST_P(set_test, union_of_basic_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(1);
        Set set2;
        set2.emplace(2);
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_basic_parallel_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(1);
        Set set2;
        set2.emplace(2);
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
    }

    TYPED_TEST_P(set_test, union_of_conflict_basic_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(1);
        Set set2;
        set2.emplace(1);
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_conflict_basic_parallel_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(1);
        Set set2;
        set2.emplace(1);
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
    }

    TYPED_TEST_P(set_test, union_of_empty_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(1);
        Set set2;
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
        union_of_test_template(this -> tracker, this -> executor, set2, set1, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_intermediate_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        Set set1(stubs.begin(), stubs.begin() + SMALL_LIMIT / 2);
        Set set2(stubs.begin() + SMALL_LIMIT / 2, stubs.end());
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_intermediate_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        Set set1(stubs.begin(), stubs.begin() + SMALL_LIMIT / 2);
        Set set2(stubs.begin() + SMALL_LIMIT / 2, stubs.end());
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
    }
    
    TYPED_TEST_P(set_test, union_of_conflict_intermediate_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin(), stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.end());
        union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_stress_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1;
        Set set2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 2; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, union_of_stress_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1;
        Set set2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 2; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, union_of_conflict_stress_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, union_of_conflict_stress_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                union_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_basic_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(0);
        set1.emplace(1);
        Set set2;
        set2.emplace(0);
        set2.emplace(2);
        intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, intersection_of_basic_parallel_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(0);
        set1.emplace(1);
        Set set2;
        set2.emplace(0);
        set2.emplace(2);
        intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
    }

    TYPED_TEST_P(set_test, intersection_of_empty_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(0);
        Set set2;
        set2.emplace(1);
        intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
        intersection_of_test_template(this -> tracker, this -> executor, set2, set1, uid_resolver());
    }

    TYPED_TEST_P(set_test, intersection_of_intermediate_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_intermediate_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_stress_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_stress_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                intersection_of_test_template(this -> tracker, this -> executor, set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, difference_of_basic_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(0);
        set1.emplace(1);
        Set set2;
        set2.emplace(0);
        set1.emplace(2);
        difference_of_test_template(this -> tracker, this -> executor, set1, set2);
    }

    TYPED_TEST_P(set_test, difference_of_basic_parallel_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(0);
        set1.emplace(1);
        Set set2;
        set2.emplace(0);
        set1.emplace(2);
        difference_of_test_template(this -> tracker, this -> executor, set1, set2, true);
    }

    TYPED_TEST_P(set_test, difference_of_empty_test) {
        using Set = typename TypeParam::regular_type;
        Set set1;
        set1.emplace(0);
        Set set2;
        set2.emplace(0);
        difference_of_test_template(this -> tracker, this -> executor, set1, set2);
        difference_of_test_template(this -> tracker, this -> executor, set2, set1);
    }

    TYPED_TEST_P(set_test, difference_of_intermediate_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT * 2);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> tracker, this -> executor, set1, set2);
            }
        }
    }

    TYPED_TEST_P(set_test, difference_of_intermediate_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(SMALL_LIMIT * 2);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> tracker, this -> executor, set1, set2, true);
            }
        }
    }

    
    TYPED_TEST_P(set_test, difference_of_stress_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> tracker, this -> executor, set1, set2);
            }
        }
    }

    TYPED_TEST_P(set_test, difference_of_stress_parallel_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        Set set1(stubs.begin() + share_start, stubs.begin() + share_end);
        Set set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                difference_of_test_template(this -> tracker, this -> executor, set1, set2, true);
            }
        }
    }

    // Mixed stress tests
    TYPED_TEST_P(set_test, mixed_stress_test) {
        typename TypeParam::regular_type set;
        int SRTESS_LIMIT = 20000;

        std::set<constructor_stub, constructor_stub_comparator> stub_set;
        for (int i = 0; i < SRTESS_LIMIT; ++i) {
            int num = random_number(0, 4000);
            switch (do_action_lottery()) {
                case LOOK_UP: {
                    EXPECT_EQ(stub_set.contains(num), set.contains(num));
                    break;
                }
                case INSERT: {
                    auto expect_res = stub_set.insert(num);
                    auto actual_res = set.insert(num);
                    EXPECT_EQ(expect_res.first != stub_set.end(), actual_res.first != set.end());
                    break;
                }
                case DELETE: {
                    EXPECT_EQ(stub_set.erase(num), set.erase(num));
                    break;
                }
            }
        }
    }


    TYPED_TEST_P(set_test, copy_only_constructor_test) {
        using Set = typename TypeParam::copy_only_type;
        std::vector<copy_only_constructor_stub> stubs;
        Set set1;
        Set set2(stubs.begin(), stubs.end());
        Set set3(std::make_move_iterator(stubs.begin()), std::make_move_iterator(stubs.end()));
        Set set4(set2);
        Set set5(std::move(set2));
    }

    TYPED_TEST_P(set_test, copy_only_operation_test) {
        using Set = typename TypeParam::copy_only_type;
        std::vector<copy_only_constructor_stub> stubs(SMALL_LIMIT);
        Set set1;
        set1.insert(stubs.begin(), stubs.end());
        set1.insert(stubs[0]);
        set1.insert(std::move(stubs[0]));
        set1.emplace(SPECIAL_VALUE);
        set1.find(stubs[0]);
        set1.lower_bound(stubs[0]);
        set1.upper_bound(stubs[0]);
        for (auto& p : set1) {
            ++p.id;
        }
        set1.erase(stubs[0]);
        set1.erase(set1.begin());
        set1.erase(set1.begin(), set1.end());
        Set set2, set3;
        Set set = union_of(Set(set2), Set(set3), this -> executor, chooser<typename Set::value_type>());
        set = intersection_of(Set(set2), Set(set3), this -> executor, chooser<typename Set::value_type>());
        set = difference_of(Set(set2), Set(set3), this -> executor);
        set2.swap(set3);
        EXPECT_FALSE(set1.contains(stubs[0]));
    }

    TYPED_TEST_P(set_test, move_only_constructor_test) {
        using Set = typename TypeParam::move_only_type;
        std::vector<move_only_constructor_stub> stubs;
        Set set1;
        Set set2(std::make_move_iterator(stubs.begin()), std::make_move_iterator(stubs.end()));
        Set set3(std::move(set2));
    }

    TYPED_TEST_P(set_test, move_only_operation_test) {
        using Set = typename TypeParam::move_only_type;
        std::vector<move_only_constructor_stub> stubs(SMALL_LIMIT);
        Set set1;
        set1.insert(std::make_move_iterator(stubs.begin()), std::make_move_iterator(stubs.end()));
        set1.insert(std::move(stubs[0]));
        set1.emplace(SPECIAL_VALUE2);
        set1.find(stubs[0]);
        set1.lower_bound(stubs[0]);
        set1.upper_bound(stubs[0]);
        for (auto& p : set1) {
            ++p.id;
        }
        set1.erase(stubs[0]);
        set1.erase(set1.begin());
        set1.erase(set1.begin(), set1.end());
        Set set2 = union_of(Set(), Set(), this -> executor, chooser<typename Set::value_type>());
        set2 = intersection_of(Set(), Set(), this -> executor, chooser<typename Set::value_type>());
        set2 = difference_of(Set(), Set(), this -> executor);
        set2.swap(set1);
        EXPECT_FALSE(set1.contains(stubs[0]));
    }

    TYPED_TEST_P(set_test, equality_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1(stubs.cbegin(), stubs.cend());
        Set set2(stubs.cbegin(), stubs.cend());
        EXPECT_EQ(set1, set2);
    }

    TYPED_TEST_P(set_test, inequality_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1(stubs.cbegin(), stubs.cend());
        Set set2(stubs.cbegin(), stubs.cend() - 1);
        EXPECT_NE(set1, set2);
    }

    TYPED_TEST_P(set_test, three_way_comparison_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1(stubs.cbegin(), stubs.cend());
        stubs.back().id++;
        Set set2(stubs.cbegin(), stubs.cend());
        EXPECT_LE(set1, set2);
        EXPECT_LT(set1, set2);
        EXPECT_GT(set2, set1);
        EXPECT_GE(set2, set1);
    }

    TYPED_TEST_P(set_test, three_way_comparison_length_test) {
        using Set = typename TypeParam::regular_type;
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        Set set1(stubs.cbegin(), stubs.cend());
        set1.erase(std::prev(set1.end()));
        Set set2(stubs.cbegin(), stubs.cend());
        EXPECT_LE(set1, set2);
        EXPECT_LT(set1, set2);
        EXPECT_GT(set2, set1);
        EXPECT_GE(set2, set1);
    }
    
    REGISTER_TYPED_TEST_SUITE_P(set_test,
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
        insert_return_value_test,
        insert_find_intermediate_test,
        insert_find_stress_test,
        insert_range_test,
        insert_hint_equal_hint_test,
        insert_hint_equal_next_hint_test,
        insert_hint_equal_prev_hint_test,
        insert_hint_overshoot_hint_test,
        insert_hint_prev_bound_test,
        insert_hint_right_bound_test,
        insert_hint_undershoot_hint_test,
        insert_hint_begin_hint_test,
        insert_hint_end_hint_test,
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
        intersection_of_intermediate_parallel_test,
        intersection_of_stress_test,
        intersection_of_stress_parallel_test,
        difference_of_basic_test,
        difference_of_basic_parallel_test,
        difference_of_empty_test,
        difference_of_intermediate_test,
        difference_of_intermediate_parallel_test,
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
