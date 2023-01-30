#include "gtest/gtest.h"
#include "tree/red_black_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/tree/tree_test_util.h"
#include "tst/tree/tree_bulk_operation_complexity_test.h"
#include "tst/tree/tree_parallel_comparison_test.h"
#include "tst/utility/common.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_set>
#include <random>
#include <set>
#include <map>
#include <chrono>


namespace {
    
    using namespace algo;
    class red_black_tree_test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            constructor_stub::reset_constructor_destructor_counter();
            std::srand(7759);
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
        }
    };

    static const int SPECIAL_VALUE = 0xdeadbeef;
    static const int SRTESS_LIMIT = 100000;
    static const int LIMIT = 10000;
    static const int MEDIUM_LIMIT = 300;
    static const int SMALL_LIMIT = 10;
    static const double HEIGHT_CAP_RATIO = 1.44;

    using int_tree_type = red_black_tree<int, constructor_stub, constructor_stub_key_getter>;
    using stub_tree_type = red_black_tree<constructor_stub, constructor_stub, std::identity, constructor_stub_comparator>;
    using stub_node_type = stub_tree_type::node_type;
    using stub_ptr_type = std::unique_ptr<stub_node_type>;

    void equivalence_test(const stub_tree_type& tree, std::vector<constructor_stub>& stubs) {
            for (auto& stub : stubs) {
                EXPECT_EQ(stub, *tree.find(stub));
            }
            EXPECT_EQ(tree.size(), stubs.size());
        }

    std::vector<constructor_stub> get_random_number_vector(std::size_t size, int lo = 0, int hi = LIMIT) {
        std::unordered_set<int> ids;
        while (ids.size() < size) {
            ids.insert(random_number(lo, hi));
        }
        return std::vector<constructor_stub> (ids.cbegin(), ids.cend());
    }

    // Constructor and assignment operator tests
    TEST_F(red_black_tree_test, default_constructor_test) {
        int_tree_type tree;
    }

    TEST_F(red_black_tree_test, comparator_constructor_test) {
        constructor_stub_comparator comparator(false);
        stub_tree_type tree(comparator);
        stub_tree_type tree2(constructor_stub_comparator(false));
    }

    TEST_F(red_black_tree_test, copy_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy(tree);
        EXPECT_TRUE(tree_copy.is_valid());
        equivalence_test(tree_copy, stubs);
    }

    TEST_F(red_black_tree_test, move_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        stub_tree_type tree_move(std::move(tree));
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        equivalence_test(tree_move, stubs);
    }

    TEST_F(red_black_tree_test, copy_assign_operator_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy;
        tree_copy = tree;
        equivalence_test(tree_copy, stubs);
    }

    TEST_F(red_black_tree_test, move_assign_operator_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        stub_tree_type tree_move;
        tree_move = std::move(tree);
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        equivalence_test(tree_move, stubs);
    }

    TEST_F(red_black_tree_test, range_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        equivalence_test(tree, stubs);
    }

    TEST_F(red_black_tree_test, swap_test) {
        std::vector<constructor_stub> stubs1 = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree1(stubs1.cbegin(), stubs1.cend());
        stub_tree_type tree_copy1(tree1);
        std::vector<constructor_stub> stubs2 = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree2(stubs2.cbegin(), stubs2.cend());
        stub_tree_type tree_copy2(tree2);
        std::swap(tree1, tree2);
        equivalence_test(tree2, stubs1);
        equivalence_test(tree1, stubs2);
    }

    std::size_t compute_max_height(std::size_t height) {
        return (std::size_t) std::floor(pow((double) height, 1 / 2));
    }

    void join_test(std::size_t left_size, std::size_t right_size, bool has_middle = true) {
        std::vector<constructor_stub> negative_stubs = get_random_number_vector(left_size, -LIMIT, 1);
        stub_tree_type left(negative_stubs.begin(), negative_stubs.end());
        std::vector<constructor_stub> positive_stubs = get_random_number_vector(right_size, 1, LIMIT);
        stub_tree_type right(positive_stubs.begin(), positive_stubs.end());

        stub_node_type* left_root = nullptr;
        if (left.get_sentinel() -> left_child) {
            left_root = left.get_sentinel() -> left_child -> deep_clone();
        }

        stub_node_type* right_root = nullptr;
        if (right.get_sentinel() -> left_child) {
            right_root = right.get_sentinel() -> left_child -> deep_clone();
        }

        int default_constructor_invocation_count = constructor_stub::default_constructor_invocation_count;
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;

        stub_ptr_type result;
        if (has_middle) {
            result = stub_tree_type::join(stub_ptr_type(left_root), stub_ptr_type(new stub_node_type(0)), stub_ptr_type(right_root));
        } else {
            result = stub_tree_type::join(stub_ptr_type(left_root), stub_ptr_type(right_root));
        }
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(default_constructor_invocation_count, constructor_stub::default_constructor_invocation_count);
        EXPECT_EQ(result -> parent, nullptr);
        EXPECT_TRUE(stub_tree_type::has_no_consecutive_red_nodes_helper(result.get()));
        EXPECT_TRUE(stub_tree_type::are_all_black_paths_equally_long_helper(result.get()) != -1);

        std::sort(negative_stubs.begin(), negative_stubs.end(), constructor_stub_comparator());
        std::sort(positive_stubs.begin(), positive_stubs.end(), constructor_stub_comparator());
        std::vector<constructor_stub> all_stubs(std::move(negative_stubs));
        if (has_middle) {
            all_stubs.emplace_back(0);
        }
        all_stubs.insert(all_stubs.end(), positive_stubs.begin(), positive_stubs.end());
        stub_node_type* curr = result -> get_leftmost_descendant();
        for (auto it1 = all_stubs.begin(); it1 != all_stubs.end(); it1++) {
            EXPECT_NE(curr, nullptr);
            if (!curr) {
                break;
            }
            EXPECT_EQ((*it1).id, curr -> value.id);
            curr = curr -> next();
        }
    }

    TEST_F(red_black_tree_test, join_missing_left_basic_test) {
        join_test(0, 1);
    }

    TEST_F(red_black_tree_test, join_missing_right_basic_test) {
        join_test(1, 0);
    }

    TEST_F(red_black_tree_test, join_basic_test) {
        join_test(1, 1);
    }

    TEST_F(red_black_tree_test, join_left_taller_test) {
        join_test(SMALL_LIMIT, compute_max_height(SMALL_LIMIT));
    }

    TEST_F(red_black_tree_test, join_missing_left_intermediate_test) {
        join_test(0, SMALL_LIMIT);
    }

    TEST_F(red_black_tree_test, join_missing_right_intermediate_test) {
        join_test(SMALL_LIMIT, 0);
    }

    TEST_F(red_black_tree_test, join_left_taller_skew_test) {
        join_test(SMALL_LIMIT, 1);
    }

    TEST_F(red_black_tree_test, join_right_taller_test) {
        join_test(compute_max_height(SMALL_LIMIT), SMALL_LIMIT);
    }

    TEST_F(red_black_tree_test, join_right_taller_skew_test) {
        join_test(1, SMALL_LIMIT);
    }

    TEST_F(red_black_tree_test, join_left_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(MEDIUM_LIMIT, i);
        }
    }

    TEST_F(red_black_tree_test, join_right_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(i, MEDIUM_LIMIT);
        }
    }

    TEST_F(red_black_tree_test, join_no_middle_missing_left_basic_test) {
        join_test(0, 1, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_missing_right_basic_test) {
        join_test(1, 0, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_basic_test) {
        join_test(1, 1, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_left_taller_test) {
        join_test(SMALL_LIMIT, compute_max_height(SMALL_LIMIT), false);
    }

    TEST_F(red_black_tree_test, join_no_middle_missing_left_intermediate_test) {
        join_test(0, SMALL_LIMIT, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_missing_right_intermediate_test) {
        join_test(SMALL_LIMIT, 0, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_left_taller_skew_test) {
        join_test(SMALL_LIMIT, 1, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_right_taller_test) {
        join_test(compute_max_height(SMALL_LIMIT), SMALL_LIMIT, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_right_taller_skew_test) {
        join_test(1, SMALL_LIMIT, false);
    }

    TEST_F(red_black_tree_test, join_no_middle_left_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(MEDIUM_LIMIT, i, false);
        }
    }

    TEST_F(red_black_tree_test, join_no_middle_right_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(i, MEDIUM_LIMIT, false);
        }
    }

    template<typename Resolver>
    void split_test(const stub_tree_type& tree, constructor_stub& divider, Resolver resolver, bool has_conflict, bool keep_divider = false) {
        // The constructor stub in this node has smaller id
        stub_node_type* node = new stub_node_type(divider);
        stub_node_type* root = nullptr;
        if (tree.get_sentinel() -> left_child) {
            root = tree.get_sentinel() -> left_child -> deep_clone();
        }
        
        int default_constructor_invocation_count = constructor_stub::default_constructor_invocation_count;
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        std::pair<stub_ptr_type, bool> split_result = tree.split(stub_ptr_type(root), stub_ptr_type(node), resolver);
        stub_ptr_type split_root = std::move(split_result.first);
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(default_constructor_invocation_count, constructor_stub::default_constructor_invocation_count);
        
        EXPECT_EQ(split_root -> parent, nullptr);
        EXPECT_TRUE(stub_tree_type::has_no_consecutive_red_nodes_helper(split_root -> left_child.get()));
        EXPECT_TRUE(stub_tree_type::are_all_black_paths_equally_long_helper(split_root -> left_child.get()) != -1);
        EXPECT_TRUE(stub_tree_type::has_no_consecutive_red_nodes_helper(split_root -> right_child.get()));
        EXPECT_TRUE(stub_tree_type::are_all_black_paths_equally_long_helper(split_root -> right_child.get()) != -1);

        if (has_conflict) {
            EXPECT_EQ(split_root.get() == node, keep_divider);
        } else {
            EXPECT_EQ(split_root.get(), node);
        }
        EXPECT_EQ(split_result.second, has_conflict);
        
        stub_node_type* curr = split_root -> get_leftmost_descendant();
        for (auto it1 = tree.begin(); it1 != tree.end() && curr != nullptr; it1++) {
            if (curr == split_root.get() && ((*it1).id != curr -> value.id)) {
                curr = curr -> next();
            }
            EXPECT_EQ((*it1).id, curr -> value.id);
            curr = curr -> next();
        }
    }

    template<typename Resolver>
    void split_test(std::vector<constructor_stub>& stubs, constructor_stub& divider, Resolver resolver, bool has_conflict, bool keep_divider = false) {
        stub_tree_type tree(stubs.begin(), stubs.end());
        return split_test(tree, divider, resolver, has_conflict, keep_divider);
    }

    TEST_F(red_black_tree_test, split_basic_test) {
        std::vector<constructor_stub> stubs;
        stubs.emplace_back(-1);
        stubs.emplace_back(0);
        stubs.emplace_back(2);
        constructor_stub stub(1);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), false);
    }

    TEST_F(red_black_tree_test, split_conflict_basic_test) {
        std::vector<constructor_stub> stubs;
        stubs.emplace_back(-1);
        stubs.emplace_back(0);
        stubs.emplace_back(1);
        constructor_stub stub(0);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), true, stub.uid < stubs[1].uid);
    }

    TEST_F(red_black_tree_test, split_conflict_symmetric_basic_test) {
        std::vector<constructor_stub> stubs;
        stubs.emplace_back(-1);
        stubs.emplace_back(0);
        stubs.emplace_back(1);
        constructor_stub stub(0);
        split_test<uid_resolver>(stubs, stub, uid_resolver(false), true, stub.uid > stubs[1].uid);
    }

    TEST_F(red_black_tree_test, split_balanced_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        int mid_index = copy_stubs.size() / 2;
        constructor_stub stub((copy_stubs[mid_index].id + copy_stubs[mid_index + 1].id) / 2);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), false);
    }

    TEST_F(red_black_tree_test, split_conflict_balanced_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        int mid_index = copy_stubs.size() / 2;
        constructor_stub stub(copy_stubs[mid_index].id);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), true, stub.uid < copy_stubs[mid_index].uid);
    }
    
    TEST_F(red_black_tree_test, split_left_empty_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        constructor_stub stub(copy_stubs[0].id - 1);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), false);
    }

    TEST_F(red_black_tree_test, split_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(11);
        stub_tree_type tree(stubs.begin(), stubs.end());

        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        for (unsigned int i = 0; i < copy_stubs.size(); i++) {
            constructor_stub stub(copy_stubs[i].id + 1);
            if (i == copy_stubs.size() - 1 || stub.id != copy_stubs[i + 1].id) {
                split_test<uid_resolver>(tree, stub, uid_resolver(), false);
            }
        }
    }

    TEST_F(red_black_tree_test, split_conflict_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.begin(), stubs.end());

        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        for (unsigned int i = 0; i < copy_stubs.size(); i++) {
            constructor_stub stub(copy_stubs[i].id);
            split_test<uid_resolver>(tree, stub, uid_resolver(), true, stub.uid < copy_stubs[i].uid);
        }
    }

    TEST_F(red_black_tree_test, split_stress_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.begin(), stubs.end());
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        int skip = copy_stubs.size() / REPEAT;
        for (unsigned int i = 0; i < copy_stubs.size(); i += skip) {
            constructor_stub stub(copy_stubs[i].id + 1);
            if (i == copy_stubs.size() - 1 || stub.id != copy_stubs[i + 1].id) {
                split_test<uid_resolver>(tree, stub, uid_resolver(), false);
            }
        }
    }

    TEST_F(red_black_tree_test, split_conflict_stress_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.begin(), stubs.end());

        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        int skip = copy_stubs.size() / REPEAT;
        for (unsigned int i = 0; i < copy_stubs.size(); i += skip) {
            constructor_stub stub(copy_stubs[i].id);
            split_test<uid_resolver>(tree, stub, uid_resolver(), true, stub.uid < copy_stubs[i].uid);
        }
    }

    INSTANTIATE_TYPED_TEST_SUITE_P(red_black_tree_bulk_operation_complexity, tree_bulk_operation_complexity_test,
        stub_tree_type);
    INSTANTIATE_TYPED_TEST_SUITE_P(red_black_tree_parallel_comparison, tree_parallel_comparison_test,
        stub_tree_type);
}
