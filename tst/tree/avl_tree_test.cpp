#include "gtest/gtest.h"
#include "tree/avl_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/tree/tree_test_util.h"
#include "tst/utility/common.h"
#include "tst/tree/tree_bulk_operation_complexity_test.h"
#include "tst/tree/tree_parallel_comparison_test.h"
#include "tst/tree/tree_common_test.h"
#include "tst/utility/tracking_allocator.h"
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


    using int_tree_type = avl_tree<int, constructor_stub, constructor_stub_key_getter>;
    using default_alloc_tree_type = avl_tree<constructor_stub, constructor_stub, std::identity, constructor_stub_comparator>;
    using stub_node_type = default_alloc_tree_type::node_type;
    using stub_ptr_type = default_alloc_tree_type::unique_ptr_type;
    using stub_tree_type = avl_tree<constructor_stub, constructor_stub, std::identity, constructor_stub_comparator, tracking_allocator<constructor_stub> >;


    class avl_tree_test : public ::testing::Test {
    protected:
        static void SetUpTestCase() {
            std::srand(7759);
        }

        virtual void SetUp() {
            tracking_allocator<stub_node_type>::reset();
            tracking_allocator<constructor_stub>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<stub_node_type>::check();
            tracking_allocator<constructor_stub>::check();
        }
    };

    template <typename T>
    std::size_t is_height_correct_helper(avl_node<T>* node) {
        if (!node) {
            return 0;
        }
        std::size_t left_height = is_height_correct_helper(node -> left_child.get());
        std::size_t right_height = is_height_correct_helper(node -> right_child.get());
        std::size_t height_diff;
        if (left_height < right_height) {
            height_diff = right_height - left_height;
        } else {
            height_diff = left_height - right_height;
        }
        // height diff must be <= 1
        EXPECT_LE(height_diff, 1);
        char expected_height = std::max(left_height, right_height) + 1;
        EXPECT_EQ(node -> height, expected_height);
        return expected_height;
    }

    void print_stub_node(stub_node_type* node) {
        if (!node) {
            return;
        }
        print_stub_node(node -> left_child.get());
        print_stub_node(node -> right_child.get());
        std::cout << node -> value.id << ": ";
        if (node -> left_child) {
            std::cout << node -> left_child -> value.id << " ";
            if (node -> left_child -> parent != node) {
                std::cout << "bad left child";
            }
        }
        if (node -> right_child) {
            std::cout << node -> right_child -> value.id << " ";
            if (node -> right_child -> parent != node) {
                std::cout << "bad right child";
            }
        }
        std::cout << std::endl;
    }

    std::size_t compute_max_height(std::size_t height) {
        return (std::size_t) std::floor(pow((double) height, 1 / 1.44));
    }

    void join_test(std::size_t left_size, std::size_t right_size, bool has_middle = true) {
        static tracking_allocator<stub_node_type> node_allocator;
        std::vector<constructor_stub> negative_stubs = get_random_stub_vector(left_size, -LIMIT, 1);
        stub_tree_type left(negative_stubs.begin(), negative_stubs.end());
        std::vector<constructor_stub> positive_stubs = get_random_stub_vector(right_size, 1, LIMIT);
        stub_tree_type right(positive_stubs.begin(), positive_stubs.end());

        stub_node_type* left_root = nullptr;
        if (left.__get_sentinel() -> left_child) {
            left_root = left.__get_sentinel() -> left_child -> deep_clone(node_allocator);
        }

        stub_node_type* right_root = nullptr;
        if (right.__get_sentinel() -> left_child) {
            right_root = right.__get_sentinel() -> left_child -> deep_clone(node_allocator);
        }

        int default_constructor_invocation_count = constructor_stub::default_constructor_invocation_count;
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;

        stub_ptr_type result;
        if (has_middle) {
            result = stub_tree_type::join(stub_ptr_type(left_root), stub_ptr_type(stub_node_type::construct(node_allocator, 0)), stub_ptr_type(right_root));
        } else {
            result = stub_tree_type::join(stub_ptr_type(left_root), stub_ptr_type(right_root));
        }
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(default_constructor_invocation_count, constructor_stub::default_constructor_invocation_count);
        EXPECT_EQ(result -> parent, nullptr);
        is_height_correct_helper(result.get());

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
        result -> deep_destroy(node_allocator);
        EXPECT_GE(allocated<stub_node_type>, left_size + right_size);
    }

    TEST_F(avl_tree_test, join_missing_left_basic_test) {
        join_test(0, 1);
    }

    TEST_F(avl_tree_test, join_missing_right_basic_test) {
        join_test(1, 0);
    }

    TEST_F(avl_tree_test, join_basic_test) {
        join_test(1, 1);
    }

    TEST_F(avl_tree_test, join_left_taller_test) {
        join_test(SMALL_LIMIT, compute_max_height(SMALL_LIMIT));
    }

    TEST_F(avl_tree_test, join_missing_left_intermediate_test) {
        join_test(0, SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, join_missing_right_intermediate_test) {
        join_test(SMALL_LIMIT, 0);
    }

    TEST_F(avl_tree_test, join_left_taller_skew_test) {
        join_test(SMALL_LIMIT, 1);
    }

    TEST_F(avl_tree_test, join_right_taller_test) {
        join_test(compute_max_height(SMALL_LIMIT), SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, join_right_taller_skew_test) {
        join_test(1, SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, join_left_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(MEDIUM_LIMIT, i);
        }
    }

    TEST_F(avl_tree_test, join_right_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(i, MEDIUM_LIMIT);
        }
    }

    TEST_F(avl_tree_test, join_no_middle_missing_left_basic_test) {
        join_test(0, 1, false);
    }

    TEST_F(avl_tree_test, join_no_middle_missing_right_basic_test) {
        join_test(1, 0, false);
    }

    TEST_F(avl_tree_test, join_no_middle_basic_test) {
        join_test(1, 1, false);
    }

    TEST_F(avl_tree_test, join_no_middle_left_taller_test) {
        join_test(SMALL_LIMIT, compute_max_height(SMALL_LIMIT), false);
    }

    TEST_F(avl_tree_test, join_no_middle_missing_left_intermediate_test) {
        join_test(0, SMALL_LIMIT, false);
    }

    TEST_F(avl_tree_test, join_no_middle_missing_right_intermediate_test) {
        join_test(SMALL_LIMIT, 0, false);
    }

    TEST_F(avl_tree_test, join_no_middle_left_taller_skew_test) {
        join_test(SMALL_LIMIT, 1, false);
    }

    TEST_F(avl_tree_test, join_no_middle_right_taller_test) {
        join_test(compute_max_height(SMALL_LIMIT), SMALL_LIMIT, false);
    }

    TEST_F(avl_tree_test, join_no_middle_right_taller_skew_test) {
        join_test(1, SMALL_LIMIT, false);
    }

    TEST_F(avl_tree_test, join_no_middle_left_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(MEDIUM_LIMIT, i, false);
        }
    }

    TEST_F(avl_tree_test, join_no_middle_right_taller_stress_test) {
        int limit = compute_max_height(MEDIUM_LIMIT);
        int skip = MEDIUM_LIMIT / REPEAT;
        for (int i = 0; i < limit; i += skip) {
            join_test(i, MEDIUM_LIMIT, false);
        }
    }

    template<typename Resolver>
    void split_test(stub_tree_type& tree, constructor_stub& divider, Resolver resolver, bool has_conflict, bool keep_divider = false) {
        static tracking_allocator<stub_node_type> node_allocator;
        // The constructor stub in this node has smaller id
        stub_node_type* node = stub_node_type::construct(node_allocator, divider);
        stub_node_type* root = nullptr;
        if (tree.__get_sentinel() -> left_child) {
            root = tree.__get_sentinel() -> left_child -> deep_clone(node_allocator);
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
        is_height_correct_helper(split_root -> left_child.get());
        is_height_correct_helper(split_root -> right_child.get());


        EXPECT_EQ(split_result.second, has_conflict);
        if (has_conflict) {
            EXPECT_EQ(split_root.get() == node, keep_divider);
        } else {
            EXPECT_EQ(split_root.get(), node);
        }
        if (split_root -> left_child) {
            EXPECT_EQ(split_root -> left_child -> parent, nullptr);
            split_root -> left_child -> parent = split_root.get();
        }
        if (split_root -> right_child) {
            EXPECT_EQ(split_root -> right_child -> parent, nullptr);
            split_root -> right_child -> parent = split_root.get();
        }
        stub_node_type* curr = split_root -> get_leftmost_descendant();
        for (auto it1 = tree.begin(); it1 != tree.end() && curr != nullptr; it1++) {
            if (curr == split_root.get() && ((*it1).id != curr -> value.id)) {
                curr = curr -> next();
            }
            EXPECT_EQ((*it1).id, curr -> value.id);
            curr = curr -> next();
        }
        split_root.release() -> deep_destroy(node_allocator);
        EXPECT_GE(allocated<stub_node_type>, 1);
    }

    template<typename Resolver>
    void split_test(std::vector<constructor_stub>& stubs, constructor_stub& divider, Resolver resolver, bool has_conflict, bool keep_divider = false) {
        stub_tree_type tree(stubs.begin(), stubs.end());
        split_test(tree, divider, resolver, has_conflict, keep_divider);
    }

    TEST_F(avl_tree_test, split_basic_test) {
        std::vector<constructor_stub> stubs;
        stubs.emplace_back(-1);
        stubs.emplace_back(0);
        stubs.emplace_back(2);
        constructor_stub stub(1);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), false);
    }

    TEST_F(avl_tree_test, split_conflict_basic_test) {
        std::vector<constructor_stub> stubs;
        stubs.emplace_back(-1);
        stubs.emplace_back(0);
        stubs.emplace_back(1);
        constructor_stub stub(0);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), true, stub.uid < stubs[1].uid);
    }

    TEST_F(avl_tree_test, split_conflict_symmetric_basic_test) {
        std::vector<constructor_stub> stubs;
        stubs.emplace_back(-1);
        stubs.emplace_back(0);
        stubs.emplace_back(1);
        constructor_stub stub(0);
        split_test<uid_resolver>(stubs, stub, uid_resolver(false), true, stub.uid > stubs[1].uid);
    }

    TEST_F(avl_tree_test, split_balanced_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        int mid_index = copy_stubs.size() / 2;
        constructor_stub stub((copy_stubs[mid_index].id + copy_stubs[mid_index + 1].id) / 2);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), false);
    }

    TEST_F(avl_tree_test, split_conflict_balanced_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        int mid_index = copy_stubs.size() / 2;
        constructor_stub stub(copy_stubs[mid_index].id);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), true, stub.uid < copy_stubs[mid_index].uid);
    }
    
    TEST_F(avl_tree_test, split_left_empty_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        constructor_stub stub(copy_stubs[0].id - 1);
        split_test<uid_resolver>(stubs, stub, uid_resolver(), false);
    }

    TEST_F(avl_tree_test, split_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(11);
        stub_tree_type tree(stubs.begin(), stubs.end());

        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        for (std::size_t i = 0; i < copy_stubs.size(); ++i) {
            constructor_stub stub(copy_stubs[i].id + 1);
            if (i == copy_stubs.size() - 1 || stub.id != copy_stubs[i + 1].id) {
                split_test<uid_resolver>(tree, stub, uid_resolver(), false);
            }
        }
    }

    TEST_F(avl_tree_test, split_conflict_intermediate_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(SMALL_LIMIT);
        stub_tree_type tree(stubs.begin(), stubs.end());

        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        for (std::size_t i = 0; i < copy_stubs.size(); ++i) {
            constructor_stub stub(copy_stubs[i].id);
            split_test<uid_resolver>(tree, stub, uid_resolver(), true, stub.uid < copy_stubs[i].uid);
        }
    }

    TEST_F(avl_tree_test, split_stress_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.begin(), stubs.end());
        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        std::size_t skip = copy_stubs.size() / REPEAT;
        for (std::size_t i = 0; i < copy_stubs.size(); i += skip) {
            constructor_stub stub(copy_stubs[i].id + 1);
            if (i == copy_stubs.size() - 1 || stub.id != copy_stubs[i + 1].id) {
                split_test<uid_resolver>(tree, stub, uid_resolver(), false);
            }
        }
    }

    TEST_F(avl_tree_test, split_conflict_stress_test) {
        std::vector<constructor_stub> stubs = get_random_stub_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.begin(), stubs.end());

        std::vector<constructor_stub> copy_stubs(stubs);
        std::sort(copy_stubs.begin(), copy_stubs.end(), constructor_stub_comparator());
        std::size_t skip = copy_stubs.size() / REPEAT;
        for (std::size_t i = 0; i < copy_stubs.size(); i += skip) {
            constructor_stub stub(copy_stubs[i].id);
            split_test<uid_resolver>(tree, stub, uid_resolver(), true, stub.uid < copy_stubs[i].uid);
        }
    }

    INSTANTIATE_TYPED_TEST_SUITE_P(avl_tree, tree_common_test, stub_tree_type);
    INSTANTIATE_TYPED_TEST_SUITE_P(avl_tree, tree_bulk_operation_complexity_test, stub_tree_type);
    INSTANTIATE_TYPED_TEST_SUITE_P(avl_tree, tree_parallel_comparison_test, stub_tree_type);
}
