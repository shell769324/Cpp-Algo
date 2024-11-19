#pragma once
#include <concepts>
#include "gtest/gtest.h"
#include "utility/constructor_stub.h"
#include "utility/common.h"
#include "utility/tracking_allocator.h"

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
        typename T::allocator_type allocator;
        protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            tracking_allocator<typename T::node_type>::reset();
            tracking_allocator<typename T::value_type>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<typename T::node_type>::check();
            tracking_allocator<typename T::value_type>::check();
        }
        public:
        static void equivalence_test(const T& set, std::vector<typename T::value_type>& stubs) {
            for (auto& stub : stubs) {
                EXPECT_TRUE(set.contains(stub));
                EXPECT_EQ(stub, *set.find(stub));
            }
            EXPECT_EQ(set.size(), stubs.size());
        }

        template<typename Set>
        static void begin_end_test_template() {
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
        static void rbegin_rend_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            Set set(stubs.begin(), stubs.end());
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

        static void insert_find_test(std::size_t size) {
            T set;
            auto stubs = get_random_stub_vector(size);
            int i = 0;
            unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
            for (auto& stub : stubs) {
                mark_constructor_counts();
                set.insert(stub);
                check_constructor_counts(0, 1, 0);
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

        static void erase_by_key(std::size_t size) {
            auto stubs = get_random_stub_vector(size);
            T set(stubs.cbegin(), stubs.cend());
            std::size_t curr_size = size;
            unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
            for (auto& stub : stubs) {
                EXPECT_NE(set.find(stub), set.end());
                mark_constructor_counts();
                set.erase(stub);
                check_constructor_counts();
                --curr_size;
                if (curr_size % skip == 0 && size == 10) {
                    EXPECT_TRUE(set.__is_valid());
                }
            }
            EXPECT_TRUE(set.empty());
        }

        static void erase_by_iterator(std::size_t size) {
            auto stubs = get_random_stub_vector(size);
            T set(stubs.cbegin(), stubs.cend());
            std::size_t curr_size = size;
            typename T::iterator it = set.begin();
            unsigned skip = std::max(size / REPEAT, (std::size_t) 1);
            while (!set.empty()) {
                typename T::value_type stub = *it;
                EXPECT_NE(set.find(stub), set.end());
                mark_constructor_counts();
                it = set.erase(it);
                --curr_size;
                check_constructor_counts();
                EXPECT_EQ(set.find(stub), set.cend());
                EXPECT_EQ(set.size(), curr_size);
                if (curr_size % skip == 0) {
                    EXPECT_TRUE(set.__is_valid());
                }
            }
            EXPECT_TRUE(set.empty());
            EXPECT_EQ(it, set.end());
        }

        template<typename Set>
        static void upper_bound_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            // Create gap between all numbers
            for (auto& stub : stubs) {
                stub.id *= 2;
            }
            T src(stubs.cbegin(), stubs.cend());
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
        static void lower_bound_test_template() {
            auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
            // Create gap between all numbers
            for (auto& stub : stubs) {
                stub.id *= 2;
            }
            T src(stubs.cbegin(), stubs.cend());
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

        template<typename Resolver>
        void union_of_test_template(const T& set1, const T& set2, Resolver resolver, bool is_parallel=false) {
            T set_copy1(set1);
            T set_copy2(set2);
            T set;
            mark_constructor_counts();
            if (is_parallel) {
                set = union_of<Resolver>(std::move(set_copy1), std::move(set_copy2), this -> executor, resolver);
            } else {
                set = union_of<Resolver>(std::move(set_copy1), std::move(set_copy2), resolver);
            }
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

        template<typename Resolver>
        void intersection_of_test_template(const T& set1, const T& set2, Resolver resolver, bool is_parallel=false) {
            T set_copy1(set1);
            T set_copy2(set2);
            T set;
            mark_constructor_counts();
            if (is_parallel) {
                set = intersection_of<Resolver>(std::move(set_copy1), std::move(set_copy2), this -> executor, resolver);
            } else {
                set = intersection_of<Resolver>(std::move(set_copy1), std::move(set_copy2), resolver);
            }
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

        void difference_of_test_template(const T& set1, const T& set2, bool is_parallel=false) {
            T set_copy1(set1);
            T set_copy2(set2);
            T set;
            mark_constructor_counts();
            if (is_parallel) {
                set = difference_of(std::move(set_copy1), std::move(set_copy2), this -> executor);
            } else {
                set = difference_of(std::move(set_copy1), std::move(set_copy2));
            }
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

        static void print_set(const T& set) {
            std::cout << "Printing set of size " << set.size() << std::endl;
            for (const auto& stub : set) {
                std::cout << stub.id << std::endl;
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


    TYPED_TEST_SUITE_P(set_test);

    TYPED_TEST_P(set_test, trait_test) {
        using const_node_type = constify<constructor_stub, typename TypeParam::node_type>;
        bool correct = std::is_same_v<typename TypeParam::key_type, constructor_stub>
            && std::is_same_v<typename TypeParam::value_type, constructor_stub>
            && std::is_same_v<typename TypeParam::size_type, std::size_t>
            && std::is_same_v<typename TypeParam::difference_type, std::ptrdiff_t>
            && std::is_same_v<typename TypeParam::value_compare, constructor_stub_comparator>
            && std::is_same_v<typename TypeParam::allocator_type, tracking_allocator<constructor_stub> >
            && std::is_same_v<typename TypeParam::reference, constructor_stub&>
            && std::is_same_v<typename TypeParam::const_reference, const constructor_stub&>
            && std::is_same_v<typename TypeParam::pointer, constructor_stub*>
            && std::is_same_v<typename TypeParam::const_pointer, const constructor_stub*>
            && std::is_same_v<typename TypeParam::iterator, binary_tree_iterator<typename TypeParam::node_type, false> >
            && std::is_same_v<typename TypeParam::const_iterator, binary_tree_iterator<const_node_type, false> >
            && std::is_same_v<typename TypeParam::reverse_iterator, binary_tree_iterator<typename TypeParam::node_type, true> >
            && std::is_same_v<typename TypeParam::const_reverse_iterator, binary_tree_iterator<const_node_type, true> >;
        EXPECT_TRUE(correct);
    }
    
    template <typename Set>
    void default_constructor_test_helper(Set& set) {
        EXPECT_TRUE(set.__is_valid());
        EXPECT_EQ(constructor_stub::constructor_invocation_count, 0);
        EXPECT_EQ(allocated<typename Set::node_type>, 1);
    }

    TYPED_TEST_P(set_test, default_constructor_test) {
        TypeParam set;
        default_constructor_test_helper(set);
    }

    TYPED_TEST_P(set_test, allocator_constructor_test) {
        TypeParam set(this -> allocator);
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
        TypeParam set(comp, this -> allocator);
        comp_allocator_constructor_test_helper(set, comp);
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    TYPED_TEST_P(set_test, comp_allocator_constructor_optional_test) {
        constructor_stub_comparator comp(true);
        TypeParam set(comp);
        comp_allocator_constructor_test_helper(set, comp);
    }

    template <typename Set, typename Stubs>
    void comp_allocator_range_constructor_test_helper(Set& set, Stubs& stubs) {
        set_test<Set>::equivalence_test(set, stubs);
        EXPECT_EQ(allocated<typename Set::node_type>, stubs.size() + 1);
    }

    TYPED_TEST_P(set_test, comp_allocator_range_constructor_test) {
        constructor_stub_comparator comp;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set(stubs.begin(), stubs.end(), comp, this -> allocator);
        comp_allocator_range_constructor_test_helper(set, stubs);
        EXPECT_EQ(comp, set.value_comp());
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    TYPED_TEST_P(set_test, comp_allocator_range_constructor_optional_allocator_test) {
        constructor_stub_comparator comp;
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set(stubs.begin(), stubs.end(), comp);
        comp_allocator_range_constructor_test_helper(set, stubs);
        EXPECT_EQ(comp, set.value_comp());
    }

    TYPED_TEST_P(set_test, comp_allocator_range_constructor_both_optional_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set(stubs.begin(), stubs.end());
        comp_allocator_range_constructor_test_helper(set, stubs);
    }

    TYPED_TEST_P(set_test, allocator_range_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set(stubs.begin(), stubs.end(), this -> allocator);
        comp_allocator_range_constructor_test_helper(set, stubs);
        EXPECT_EQ(this -> allocator, set.get_allocator());
    }

    TYPED_TEST_P(set_test, copy_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam set(comp);
        set.insert(stubs.begin(), stubs.end());
        EXPECT_TRUE(set.__is_valid());
        std::size_t alloc_count = allocated<typename TypeParam::node_type>;
        TypeParam set_copy(set);
        EXPECT_EQ(alloc_count + SMALL_LIMIT + 1, allocated<typename TypeParam::node_type>);
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        this -> equivalence_test(set_copy, stubs);
        EXPECT_EQ(set.get_allocator(), set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, allocator_copy_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam set(comp);
        set.insert(stubs.begin(), stubs.end());
        std::size_t alloc_count = allocated<typename TypeParam::node_type>;
        TypeParam set_copy(set, this -> allocator);
        EXPECT_EQ(alloc_count + SMALL_LIMIT + 1, allocated<typename TypeParam::node_type>);
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        this -> equivalence_test(set_copy, stubs);
        EXPECT_EQ(this -> allocator, set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, move_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam set(comp, this -> allocator);
        set.insert(stubs.begin(), stubs.end());
        mark_constructor_counts();
        TypeParam set_copy(std::move(set));
        check_constructor_counts();
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        this -> equivalence_test(set_copy, stubs);
        EXPECT_EQ(this -> allocator, set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, allocator_move_constructor_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam set(comp);
        set.insert(stubs.begin(), stubs.end());
        mark_constructor_counts();
        TypeParam set_copy(std::move(set), this -> allocator);
        check_constructor_counts();
        EXPECT_TRUE(set_copy.__is_valid());
        EXPECT_TRUE(set_copy.value_comp().reverse);
        this -> equivalence_test(set_copy, stubs);
        EXPECT_EQ(this -> allocator, set_copy.get_allocator());
    }

    TYPED_TEST_P(set_test, assignment_operator_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam set1(comp);
        set1.insert(stubs.begin(), stubs.end());
        TypeParam set2;
        set2 = set1;
        EXPECT_TRUE(set2.__is_valid());
        this -> equivalence_test(set2, stubs);
    }

    TYPED_TEST_P(set_test, move_assignment_operator_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        constructor_stub_comparator comp(true);
        TypeParam set1(comp);
        set1.insert(stubs.begin(), stubs.end());
        TypeParam set2;
        mark_constructor_counts();
        set2 = std::move(set1);
        check_constructor_counts();
        EXPECT_TRUE(set2.__is_valid());
        this -> equivalence_test(set2, stubs);
    }

    // Iterator tests

    TYPED_TEST_P(set_test, begin_end_test) {
        this -> template begin_end_test_template<TypeParam>();
    }

    TYPED_TEST_P(set_test, const_begin_end_test) {
        this -> template begin_end_test_template<const TypeParam>();
    }

    TYPED_TEST_P(set_test, cbegin_cend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        const TypeParam set(stubs.begin(), stubs.end());
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
        this -> template rbegin_rend_test_template<TypeParam>();
    }

    TYPED_TEST_P(set_test, const_rbegin_rend_test) {
        this -> template rbegin_rend_test_template<const TypeParam>();
    }

    TYPED_TEST_P(set_test, crbegin_crend_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        const TypeParam set(stubs.begin(), stubs.end());
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
        TypeParam set;
        EXPECT_TRUE(set.empty());
        set.emplace(SPECIAL_VALUE);
        EXPECT_FALSE(set.empty());
    }

    TYPED_TEST_P(set_test, size_test) {
        TypeParam set;
        EXPECT_EQ(set.size(), 0);
        set.emplace(SPECIAL_VALUE);
        EXPECT_EQ(set.size(), 1);
    }

    TYPED_TEST_P(set_test, clear_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set(stubs.begin(), stubs.end());
        set.clear();
        EXPECT_TRUE(set.empty());
    }
    
    // Core methods (find and insert) tests
    TYPED_TEST_P(set_test, const_ref_insert_find_basic_test) {
        typename TypeParam::value_type stub(SPECIAL_VALUE);
        TypeParam set;
        EXPECT_EQ(set.find(stub), set.cend());
        const typename TypeParam::value_type& const_ref = stub;
        mark_constructor_counts();
        set.insert(const_ref);
        check_constructor_counts(0, 1, 0);
        EXPECT_TRUE(set.contains(stub));
    }

    TYPED_TEST_P(set_test, rvalue_insert_find_basic_test) {
        constructor_stub stub(SPECIAL_VALUE);
        TypeParam set;
        EXPECT_EQ(set.find(stub), set.cend());
        mark_constructor_counts();
        set.insert(std::move(stub));
        check_constructor_counts(0, 0, 1);
        EXPECT_EQ(set.find(stub) -> id, SPECIAL_VALUE);
        stub = constructor_stub(SPECIAL_VALUE);
        mark_constructor_counts();
        set.insert(std::move(stub));
        check_constructor_counts();
    }

    TYPED_TEST_P(set_test, insert_return_value_test) {
        TypeParam set;
        constructor_stub value(-SPECIAL_VALUE);
        std::pair<typename TypeParam::iterator, bool> res = set.insert(value);
        EXPECT_EQ(*(res.first), value);
        EXPECT_TRUE(res.second);
        std::pair<typename TypeParam::iterator, bool> res2 = set.insert(value);
        EXPECT_EQ(*(res2.first), value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(set.size(), 1);
    }


    TYPED_TEST_P(set_test, insert_find_intermediate_test) {
        this -> insert_find_test(SMALL_LIMIT);
    }

    TYPED_TEST_P(set_test, insert_find_stress_test) {
        this -> insert_find_test(MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, insert_range_test) {
        std::vector<typename TypeParam::value_type> stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set(stubs.begin(), stubs.end());
        for (unsigned i = 0; i < stubs.size(); ++i) {
            EXPECT_TRUE(set.contains(stubs[i]));
        }
        EXPECT_TRUE(set.__is_valid());
    }

    TYPED_TEST_P(set_test, emplace_lvalue_test) {
        TypeParam set;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        set.emplace(stub);
        check_constructor_counts(0, 1, 0);
        EXPECT_TRUE(set.contains(stub));
    }

    TYPED_TEST_P(set_test, emplace_rvalue_test) {
        TypeParam set;
        constructor_stub stub(SPECIAL_VALUE);
        mark_constructor_counts();
        set.emplace(std::move(stub));
        check_constructor_counts(0, 0, 1);
        EXPECT_TRUE(set.contains(stub));
    }

    TYPED_TEST_P(set_test, emplace_id_forward_test) {
        TypeParam set;
        mark_constructor_counts();
        set.emplace(SPECIAL_VALUE);
        check_constructor_counts();
        EXPECT_TRUE(set.contains(SPECIAL_VALUE));
    }

    TYPED_TEST_P(set_test, emplace_return_test) {
        TypeParam set;
        constructor_stub value(SPECIAL_VALUE);
        std::pair<typename TypeParam::iterator, bool> res = set.emplace(value);
        EXPECT_EQ(*res.first, value);
        EXPECT_TRUE(res.second);
        std::pair<typename TypeParam::iterator, bool> res2 = set.emplace(value);
        EXPECT_EQ(*res2.first, value);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(set.size(), 1);
    }

    TYPED_TEST_P(set_test, erase_by_key_basic_test) {
        TypeParam set;
        set.emplace(SPECIAL_VALUE);
        mark_constructor_counts();
        set.erase(SPECIAL_VALUE);
        check_constructor_counts();
        EXPECT_EQ(set.find(SPECIAL_VALUE), set.cend());
    }

    TYPED_TEST_P(set_test, erase_by_key_return_value_test) {
        TypeParam set;
        set.emplace(SPECIAL_VALUE);
        EXPECT_EQ(set.erase(SPECIAL_VALUE), 1);
        EXPECT_EQ(set.erase(SPECIAL_VALUE), 0);
    }

    TYPED_TEST_P(set_test, erase_by_key_intermediate_test) {
        this -> erase_by_key(SMALL_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_by_key_stress_test) {
        this -> erase_by_key(MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_by_iterator_basic_test) {
        TypeParam set;
        typename TypeParam::iterator it = set.emplace(SPECIAL_VALUE).first;
        mark_constructor_counts();
        set.erase(it);
        check_constructor_counts();
        EXPECT_FALSE(set.contains(SPECIAL_VALUE));
    }

    TYPED_TEST_P(set_test, erase_by_iterator_return_value_test) {
        TypeParam set;
        typename TypeParam::iterator it1 = set.emplace(SPECIAL_VALUE).first;
        typename TypeParam::iterator it2 = set.emplace(SPECIAL_VALUE + 1).first;
        EXPECT_EQ(set.erase(it1), it2);
        EXPECT_EQ(set.erase(it2), set.end());
    }

    TYPED_TEST_P(set_test, erase_by_iterator_intermediate_test) {
        this -> erase_by_iterator(SMALL_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_by_iterator_stress_test) {
        this -> erase_by_iterator(MEDIUM_LIMIT);
    }

    TYPED_TEST_P(set_test, erase_range_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set(stubs.cbegin(), stubs.cend());
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
        auto stubs1 = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1(stubs1.cbegin(), stubs1.cend());
        auto stubs2 = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set2(stubs2.cbegin(), stubs2.cend());
        std::swap(set1, set2);
        this -> equivalence_test(set2, stubs1);
        this -> equivalence_test(set1, stubs2);
    }

    TYPED_TEST_P(set_test, contains_test) {
        TypeParam set;
        set.emplace(SPECIAL_VALUE);
        mark_constructor_counts();
        EXPECT_TRUE(set.contains(SPECIAL_VALUE));
        EXPECT_FALSE(set.contains(-SPECIAL_VALUE));
        check_constructor_counts();
    }

    // Upper bound and lower bound tests
    TYPED_TEST_P(set_test, upper_bound_test) {
        this -> template upper_bound_test_template<TypeParam>();
    }

    TYPED_TEST_P(set_test, upper_bound_const_test) {
        this -> template upper_bound_test_template<const TypeParam>();
    }

    TYPED_TEST_P(set_test, lower_bound_test) {
        this -> template lower_bound_test_template<TypeParam>();
    }

    TYPED_TEST_P(set_test, lower_bound_const_test) {
        this -> template lower_bound_test_template<const TypeParam>();
    }

    TYPED_TEST_P(set_test, union_of_basic_test) {
        TypeParam set1;
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(2);
        this -> union_of_test_template(set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_basic_parallel_test) {
        TypeParam set1;
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(2);
        this -> union_of_test_template(set1, set2, uid_resolver(), true);
    }

    TYPED_TEST_P(set_test, union_of_conflict_basic_test) {
        TypeParam set1;
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(1);
        this -> union_of_test_template(set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_conflict_basic_parallel_test) {
        TypeParam set1;
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(1);
        this -> union_of_test_template(set1, set2, uid_resolver(), true);
    }

    TYPED_TEST_P(set_test, union_of_empty_test) {
        TypeParam set1;
        set1.emplace(1);
        TypeParam set2;
        this -> union_of_test_template(set1, set2, uid_resolver());
        this -> union_of_test_template(set2, set1, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_intermediate_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set1(stubs.begin(), stubs.begin() + SMALL_LIMIT / 2);
        TypeParam set2(stubs.begin() + SMALL_LIMIT / 2, stubs.end());
        this -> union_of_test_template(set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_intermediate_parallel_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        TypeParam set1(stubs.begin(), stubs.begin() + SMALL_LIMIT / 2);
        TypeParam set2(stubs.begin() + SMALL_LIMIT / 2, stubs.end());
        this -> union_of_test_template(set1, set2, uid_resolver(), true);
    }
    
    TYPED_TEST_P(set_test, union_of_conflict_intermediate_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin(), stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.end());
        this -> union_of_test_template(set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, union_of_stress_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1;
        TypeParam set2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 2; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, union_of_stress_parallel_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1;
        TypeParam set2;
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 2; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, union_of_conflict_stress_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, union_of_conflict_stress_parallel_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            unsigned reverse_index = stubs.size() - 1 - i;
            set2.insert(stubs[reverse_index]);
            if (i % skip == 0) {
                this -> union_of_test_template(set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_basic_test) {
        TypeParam set1;
        set1.emplace(0);
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(0);
        set2.emplace(2);
        this -> intersection_of_test_template(set1, set2, uid_resolver());
    }

    TYPED_TEST_P(set_test, intersection_of_basic_parallel_test) {
        TypeParam set1;
        set1.emplace(0);
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(0);
        set2.emplace(2);
        this -> intersection_of_test_template(set1, set2, uid_resolver(), true);
    }

    TYPED_TEST_P(set_test, intersection_of_empty_test) {
        TypeParam set1;
        set1.emplace(0);
        TypeParam set2;
        set2.emplace(1);
        this -> intersection_of_test_template(set1, set2, uid_resolver());
        this -> intersection_of_test_template(set2, set1, uid_resolver());
    }

    TYPED_TEST_P(set_test, intersection_of_intermediate_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_intermediate_parallel_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_stress_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(set1, set2, uid_resolver());
            }
        }
    }

    TYPED_TEST_P(set_test, intersection_of_stress_parallel_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> intersection_of_test_template(set1, set2, uid_resolver(), true);
            }
        }
    }

    TYPED_TEST_P(set_test, difference_of_basic_test) {
        TypeParam set1;
        set1.emplace(0);
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(0);
        set1.emplace(2);
        this -> difference_of_test_template(set1, set2);
    }

    TYPED_TEST_P(set_test, difference_of_basic_parallel_test) {
        TypeParam set1;
        set1.emplace(0);
        set1.emplace(1);
        TypeParam set2;
        set2.emplace(0);
        set1.emplace(2);
        this -> difference_of_test_template(set1, set2, true);
    }

    TYPED_TEST_P(set_test, difference_of_empty_test) {
        TypeParam set1;
        set1.emplace(0);
        TypeParam set2;
        set2.emplace(0);
        this -> difference_of_test_template(set1, set2);
        this -> difference_of_test_template(set2, set1);
    }

    TYPED_TEST_P(set_test, difference_of_intermediate_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT * 2);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(set1, set2);
            }
        }
    }

    TYPED_TEST_P(set_test, difference_of_intermediate_parallel_test) {
        auto stubs = get_random_stub_vector(SMALL_LIMIT * 2);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = std::max<int>(1, stubs.size() / REPEAT);
        for (unsigned i = 0; i < stubs.size() / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(set1, set2, true);
            }
        }
    }

    
    TYPED_TEST_P(set_test, difference_of_stress_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(set1, set2);
            }
        }
    }

    TYPED_TEST_P(set_test, difference_of_stress_parallel_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        unsigned share_start = stubs.size() / 3;
        unsigned share_end = stubs.size() - stubs.size() / 3;
        TypeParam set1(stubs.begin() + share_start, stubs.begin() + share_end);
        TypeParam set2(stubs.begin() + share_start, stubs.begin() + share_end);
        unsigned skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < MEDIUM_LIMIT / 3; ++i) {
            set1.insert(stubs[i]);
            set2.insert(stubs[stubs.size() - 1 - i]);
            if (i % skip == 0) {
                this -> difference_of_test_template(set1, set2, true);
            }
        }
    }

    // Mixed stress tests
    TYPED_TEST_P(set_test, mixed_stress_test) {
        TypeParam set;
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

    TYPED_TEST_P(set_test, equality_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1(stubs.cbegin(), stubs.cend());
        TypeParam set2(stubs.cbegin(), stubs.cend());
        EXPECT_EQ(set1, set2);
    }

    TYPED_TEST_P(set_test, inequality_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1(stubs.cbegin(), stubs.cend());
        TypeParam set2(stubs.cbegin(), stubs.cend() - 1);
        EXPECT_NE(set1, set2);
    }

    TYPED_TEST_P(set_test, three_way_comparison_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1(stubs.cbegin(), stubs.cend());
        stubs.back().id++;
        TypeParam set2(stubs.cbegin(), stubs.cend());
        EXPECT_LE(set1, set2);
        EXPECT_LT(set1, set2);
        EXPECT_GT(set2, set1);
        EXPECT_GE(set2, set1);
    }

    TYPED_TEST_P(set_test, three_way_comparison_length_test) {
        auto stubs = get_random_stub_vector(MEDIUM_LIMIT);
        TypeParam set1(stubs.cbegin(), stubs.cend());
        set1.erase(std::prev(set1.end()));
        TypeParam set2(stubs.cbegin(), stubs.cend());
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
        emplace_lvalue_test,
        emplace_rvalue_test,
        emplace_id_forward_test,
        emplace_return_test,
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
        equality_test,
        inequality_test,
        three_way_comparison_test,
        three_way_comparison_length_test
    );
}
