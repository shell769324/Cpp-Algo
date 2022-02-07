#include "gtest/gtest.h"
#include "tree/avl_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/stub_iterator.h"
#include "tst/tree/tree_test_util.h"
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
    class avl_tree_test : public ::testing::Test {
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
    static const int LOOK_UP = 0;
    static const int INSERT = 1;
    static const int DELETE = 2;

    using int_tree_type = avl_tree<int, constructor_stub, constructor_stub_key_getter>;
    using stub_tree_type = avl_tree<constructor_stub, constructor_stub, std::identity, constructor_stub_comparator>;
    int random_number(int lo, int hi) {
        return rand() % (hi - lo) + lo;
    }

    int random_number() {
        return random_number(0, LIMIT);
    }

    void is_equal_tree_test(const stub_tree_type& tree1, const stub_tree_type& tree2) {
        EXPECT_EQ(tree1.size(), tree2.size());
        for (const constructor_stub& val : tree1) {
            EXPECT_NE(tree2.find(val), tree2.cend());
        }
    }

    template <typename K, typename V, typename KeyOf, typename Comparator>
    void is_inorder_test(const avl_tree<K, V, KeyOf, Comparator>& tree) {
        if (tree.is_empty()) {
            return;
        }
        Comparator comp = tree.get_comparator();
        KeyOf key_of = tree.get_key_of();
        for (auto it1 = tree.cbegin(), it2 = std::next(tree.cbegin()); it2 != tree.cend(); it1++, it2++) {
            EXPECT_TRUE(comp(key_of(*it1), key_of(*it2)));
        }
    }

    template <typename T>
    std::size_t is_balance_factor_correct_helper(avl_node<T>* node) {
        if (!node) {
            return 0;
        }
        std::size_t left_height = is_balance_factor_correct_helper(node -> left_child.get());
        std::size_t right_height = is_balance_factor_correct_helper(node -> right_child.get());
        EXPECT_EQ(node -> factor, left_height - right_height);
        return std::max(left_height, right_height) + 1;
    }

    template <typename K, typename V, typename KeyOf, typename Comparator>
    void is_balance_factor_correct_test(const avl_tree<K, V, KeyOf, Comparator>& tree) {
        is_balance_factor_correct_helper(tree.get_sentinel() -> left_child.get());
    }

    std::vector<constructor_stub> get_random_number_vector(std::size_t size) {
        std::unordered_set<int> ids;
        while (ids.size() < size) {
            ids.insert(random_number());
        }
        return std::vector<constructor_stub> (ids.cbegin(), ids.cend());
    }

    double height_limit(std::size_t size) {
        double ratio = 1.44;
        return ratio * std::log2((double) size);
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

    // Constructor and assignment operator tests
    TEST_F(avl_tree_test, default_constructor_test) {
        int_tree_type tree;
    }

    TEST_F(avl_tree_test, comparator_constructor_test) {
        constructor_stub_comparator comparator(false);
        stub_tree_type tree(comparator);
        stub_tree_type tree2(constructor_stub_comparator(false));
    }

    TEST_F(avl_tree_test, copy_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy(tree);
        is_equal_tree_test(tree, tree_copy);
    }

    TEST_F(avl_tree_test, move_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        stub_tree_type tree_move(std::move(tree));
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        is_equal_tree_test(tree_move, tree_copy);
    }

    TEST_F(avl_tree_test, copy_assign_operator_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy;
        tree_copy = tree;
        is_equal_tree_test(tree, tree_copy);
    }

    TEST_F(avl_tree_test, move_assign_operator_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        stub_tree_type tree_copy(tree);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        stub_tree_type tree_move;
        tree_move = std::move(tree);
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        is_equal_tree_test(tree_move, tree_copy);
    }

    TEST_F(avl_tree_test, range_constructor_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        for (auto& val : stubs) {
            EXPECT_NE(tree.find(val), tree.cend());
        }
    }

    TEST_F(avl_tree_test, swap_test) {
        std::vector<constructor_stub> stubs1 = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree1(stubs1.cbegin(), stubs1.cend());
        stub_tree_type tree_copy1(tree1);
        std::vector<constructor_stub> stubs2 = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree2(stubs2.cbegin(), stubs2.cend());
        stub_tree_type tree_copy2(tree2);
        std::swap(tree1, tree2);
        is_equal_tree_test(tree2, tree_copy1);
        is_equal_tree_test(tree1, tree_copy2);
    }
    
    // Capacity related method tests
    TEST_F(avl_tree_test, is_empty_test) {
        stub_tree_type tree;
        EXPECT_TRUE(tree.is_empty());
        tree.insert(constructor_stub(SPECIAL_VALUE));
        EXPECT_FALSE(tree.is_empty());
    }

    TEST_F(avl_tree_test, size_test) {
        stub_tree_type tree;
        EXPECT_EQ(tree.size(), 0);
        tree.insert(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(tree.size(), 1);
    }

    // Core methods (find and insert) tests
    TEST_F(avl_tree_test, lvalue_insert_find_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        EXPECT_EQ(tree.find(stub), tree.cend());
        tree.insert(stub);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        // for lvalue constructor, we have to call move constructor when we
        // finally move the object into the newly constructed node
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ((*tree.find(stub)).id, SPECIAL_VALUE);
    }

    TEST_F(avl_tree_test, rvalue_insert_find_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        EXPECT_EQ(tree.find(stub), tree.cend());
        tree.insert(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ((*tree.find(stub)).id, SPECIAL_VALUE);
    }

    TEST_F(avl_tree_test, insert_return_value_test) {
        stub_tree_type tree;
        std::pair<stub_tree_type::iterator, bool> res = tree.insert(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ((*res.first).id, SPECIAL_VALUE);
        EXPECT_TRUE(res.second);
        std::pair<stub_tree_type::iterator, bool> res2 = tree.insert(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ((*res2.first).id, SPECIAL_VALUE);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(tree.size(), 1);
    }

    void insert_find_test(std::size_t size) {
        stub_tree_type tree;
        std::vector<constructor_stub> stubs = get_random_number_vector(size);
        int i = 0;
        for (const constructor_stub& stub : stubs) {
            int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
            int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
            tree.insert(stub);
            EXPECT_EQ(move_constructor_invocation_count + 1, constructor_stub::move_constructor_invocation_count);
            EXPECT_EQ(copy_constructor_invocation_count + 1, constructor_stub::copy_constructor_invocation_count);
            is_inorder_test(tree);
            is_balance_factor_correct_test(tree);
            EXPECT_EQ(tree.size(), i + 1);
            for (int j = 0; j <= i; j++) {
                EXPECT_EQ(*tree.find(stubs[j]), stubs[j]);
            }
            i++;
        }
    }

    TEST_F(avl_tree_test, insert_find_intermediate_test) {
        insert_find_test(SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, insert_find_stress_test) {
        insert_find_test(MEDIUM_LIMIT);
    }

    TEST_F(avl_tree_test, insert_range_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree;
        tree.insert(stubs.cbegin(), stubs.cend());
        for (auto& val : stubs) {
            EXPECT_NE(tree.find(val), tree.cend());
        }
    }

    // Upper bound and lower bound tests
    TEST_F(avl_tree_test, max_leq_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        int middle_index = stubs.size() / 2;
        // With existing element
        EXPECT_EQ(*(tree.max_leq(stubs[middle_index])), stubs[middle_index]);
        // With absent element
        int num = (stubs[middle_index].id + stubs[middle_index + 1].id) / 2;
        constructor_stub stub(num);
        EXPECT_EQ(*(tree.max_leq(stub)), stubs[middle_index]);
        EXPECT_EQ(tree.max_leq(stubs.front().id - 1), tree.end());
    }

    TEST_F(avl_tree_test, min_geq_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        int middle_index = stubs.size() / 2;
        // With existing element
        EXPECT_EQ(*(tree.min_geq(stubs[middle_index])), stubs[middle_index]);
        // With absent element
        int num = (stubs[middle_index].id + stubs[middle_index - 1].id) / 2;
        constructor_stub stub(num);
        EXPECT_EQ(*(tree.min_geq(stub)), stubs[middle_index]);
        EXPECT_EQ(tree.min_geq(stubs.back().id + 1), tree.end());
    }

    // Iterator tests
    template<typename Tree>
    void begin_end_test() {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        Tree tree(stubs.cbegin(), stubs.cend());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        for (auto& stub : tree) {
            EXPECT_EQ(stub, stubs[i]);
            i++;
        }
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TEST_F(avl_tree_test, begin_end_test) {
        begin_end_test<stub_tree_type>();
    }

    TEST_F(avl_tree_test, const_begin_end_test) {
        begin_end_test<const stub_tree_type>();
    }

    TEST_F(avl_tree_test, cbegin_cend_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        const stub_tree_type tree(stubs.cbegin(), stubs.cend());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        for (auto it = tree.cbegin(); it != tree.cend(); it++, i++) {
            EXPECT_EQ(*it, stubs[i]);
        }
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    template<typename Tree>
    void rbegin_rend_test() {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        Tree tree(stubs.cbegin(), stubs.cend());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        for (auto it = tree.rbegin(); it != tree.rend(); it++, i++) {
            EXPECT_EQ(*it, stubs[i]);
        }
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    TEST_F(avl_tree_test, rbegin_rend_test) {
        rbegin_rend_test<stub_tree_type>();
    }

    TEST_F(avl_tree_test, const_rbegin_rend_test) {
        rbegin_rend_test<const stub_tree_type>();
    }

    TEST_F(avl_tree_test, crbegin_crend_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        const stub_tree_type tree(stubs.cbegin(), stubs.cend());
        int i = 0;
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator(true));
        for (auto it = tree.crbegin(); it != tree.crend(); it++, i++) {
            EXPECT_EQ(*it, stubs[i]);
        }
        EXPECT_EQ(i, MEDIUM_LIMIT);
    }

    // Emplace tests
    TEST_F(avl_tree_test, args_emplace_find_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        EXPECT_EQ(tree.find(stub), tree.cend());
        int id_constructor_invocation_count = constructor_stub::id_constructor_invocation_count;
        tree.emplace(SPECIAL_VALUE);
        EXPECT_EQ(id_constructor_invocation_count + 1, constructor_stub::id_constructor_invocation_count);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ((*tree.find(stub)).id, SPECIAL_VALUE);
    }

    TEST_F(avl_tree_test, lvalue_emplace_find_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        EXPECT_EQ(tree.find(stub), tree.cend());
        tree.emplace(stub);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 0);
        EXPECT_EQ((*tree.find(stub)).id, SPECIAL_VALUE);
    }

    TEST_F(avl_tree_test, rvalue_emplace_find_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        EXPECT_EQ(tree.find(stub), tree.cend());
        tree.emplace(constructor_stub(SPECIAL_VALUE));
        EXPECT_EQ(constructor_stub::move_constructor_invocation_count, 1);
        EXPECT_EQ(constructor_stub::copy_constructor_invocation_count, 0);
        EXPECT_EQ((*tree.find(stub)).id, SPECIAL_VALUE);
    }

    TEST_F(avl_tree_test, emplace_return_value_test) {
        stub_tree_type tree;
        std::pair<stub_tree_type::iterator, bool> res = tree.emplace(SPECIAL_VALUE);
        EXPECT_EQ((*res.first).id, SPECIAL_VALUE);
        EXPECT_TRUE(res.second);
        int id_constructor_invocation_count = constructor_stub::id_constructor_invocation_count;
        std::pair<stub_tree_type::iterator, bool> res2 = tree.emplace(SPECIAL_VALUE);
        // emplace should create a value regardless if it already exists in the tree
        EXPECT_EQ(id_constructor_invocation_count + 1, constructor_stub::id_constructor_invocation_count);
        EXPECT_EQ((*res2.first).id, SPECIAL_VALUE);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(tree.size(), 1);
    }

    void emplace_find_test(std::size_t size) {
        stub_tree_type tree;
        std::vector<constructor_stub> stubs = get_random_number_vector(size);
        int i = 0;
        for (const constructor_stub& stub : stubs) {
            int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
            int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
            tree.emplace(stub.id);
            EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
            EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
            is_inorder_test(tree);
            is_balance_factor_correct_test(tree);
            EXPECT_EQ(tree.size(), i + 1);
            for (int j = 0; j <= i; j++) {
                EXPECT_EQ(*tree.find(stubs[j]), stubs[j]);
            }
            i++;
        }
    }

    TEST_F(avl_tree_test, emplace_find_intermediate_test) {
        emplace_find_test(SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, emplace_find_stress_test) {
        emplace_find_test(MEDIUM_LIMIT);
    }

    TEST_F(avl_tree_test, try_emplace_return_value_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        std::pair<stub_tree_type::iterator, bool> res = tree.try_emplace(stub, SPECIAL_VALUE);
        EXPECT_EQ((*res.first).id, SPECIAL_VALUE);
        EXPECT_TRUE(res.second);
        int id_constructor_invocation_count = constructor_stub::id_constructor_invocation_count;
        std::pair<stub_tree_type::iterator, bool> res2 = tree.try_emplace(stub, SPECIAL_VALUE);
        // emplace shouldn't create a value if it already exists in the tree
        EXPECT_EQ(id_constructor_invocation_count, constructor_stub::id_constructor_invocation_count);
        EXPECT_EQ((*res2.first).id, SPECIAL_VALUE);
        EXPECT_FALSE(res2.second);
        EXPECT_EQ(res.first, res2.first);
        EXPECT_EQ(tree.size(), 1);
    }

    // Erase tests
    TEST_F(avl_tree_test, erase_by_key_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        tree.insert(stub);
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        tree.erase(stub);
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(tree.find(SPECIAL_VALUE), tree.cend());
    }

    TEST_F(avl_tree_test, erase_by_key_return_value_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        tree.insert(stub);
        EXPECT_EQ(tree.erase(stub), 1);
        EXPECT_EQ(tree.erase(stub), 0);
    }

    void erase_by_key(std::size_t size) {
        std::vector<constructor_stub> stubs = get_random_number_vector(size);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        std::size_t curr_size = size;
        for (const constructor_stub& stub : stubs) {
            EXPECT_EQ(*tree.find(stub), stub);
            int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
            int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
            tree.erase(stub);
            curr_size--;
            EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
            EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
            EXPECT_EQ(tree.find(stub), tree.cend());
            EXPECT_EQ(tree.size(), curr_size);
            is_inorder_test(tree);
            is_balance_factor_correct_test(tree);
        }
        EXPECT_TRUE(tree.is_empty());
    }

    TEST_F(avl_tree_test, erase_by_key_intermediate_test) {
        erase_by_key(SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, erase_by_key_stress_test) {
        erase_by_key(MEDIUM_LIMIT);
    }

    TEST_F(avl_tree_test, erase_by_iterator_basic_test) {
        stub_tree_type tree;
        constructor_stub stub(SPECIAL_VALUE);
        stub_tree_type::iterator it = tree.insert(stub).first;
        int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
        int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
        tree.erase(it);
        EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
        EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
        EXPECT_EQ(tree.find(SPECIAL_VALUE), tree.cend());
    }

    TEST_F(avl_tree_test, erase_by_iterator_return_value_test) {
        stub_tree_type tree;
        stub_tree_type::iterator it1 = tree.insert(SPECIAL_VALUE).first;
        stub_tree_type::iterator it2 = tree.insert(SPECIAL_VALUE + 1).first;
        EXPECT_EQ(tree.erase(it1), it2);
        EXPECT_EQ(tree.erase(it2), tree.end());
    }

    void erase_by_iterator(std::size_t size) {
        std::vector<constructor_stub> stubs = get_random_number_vector(size);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        std::size_t curr_size = size;
        stub_tree_type::iterator it = tree.begin();
        while (!tree.is_empty()) {
            constructor_stub stub = *it;
            EXPECT_EQ(*tree.find(stub), stub);
            int move_constructor_invocation_count = constructor_stub::move_constructor_invocation_count;
            int copy_constructor_invocation_count = constructor_stub::copy_constructor_invocation_count;
            it = tree.erase(it);
            curr_size--;
            EXPECT_EQ(move_constructor_invocation_count, constructor_stub::move_constructor_invocation_count);
            EXPECT_EQ(copy_constructor_invocation_count, constructor_stub::copy_constructor_invocation_count);
            EXPECT_EQ(tree.find(stub), tree.cend());
            EXPECT_EQ(tree.size(), curr_size);
            is_inorder_test(tree);
            is_balance_factor_correct_test(tree);
        }
        EXPECT_EQ(it, tree.end());
    }

    TEST_F(avl_tree_test, erase_by_iterator_intermediate_test) {
        erase_by_iterator(SMALL_LIMIT);
    }

    TEST_F(avl_tree_test, erase_by_iterator_stress_test) {
        erase_by_iterator(MEDIUM_LIMIT);
    }

    TEST_F(avl_tree_test, erase_range_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(MEDIUM_LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        std::sort(stubs.begin(), stubs.end(), constructor_stub_comparator());
        std::size_t trisect1 = MEDIUM_LIMIT / 3;
        std::size_t trisect2 = MEDIUM_LIMIT - MEDIUM_LIMIT / 3;
        auto it1 = tree.find(stubs[trisect1]);
        auto it2 = tree.find(stubs[trisect2]);
        EXPECT_EQ(tree.erase(it1, it2), it2);
        EXPECT_EQ(tree.size(), MEDIUM_LIMIT - (trisect2 - trisect1));
        for (std::size_t i = trisect1; i < trisect2; i++) {
            EXPECT_EQ(tree.find(stubs[i]), tree.end());
        }
        tree.erase(it2, tree.end());
        EXPECT_EQ(tree.size(), trisect1);
        for (std::size_t i = trisect2; i < stubs.size(); i++) {
            EXPECT_EQ(tree.find(stubs[i]), tree.end());
        }
        tree.erase(tree.begin(), tree.end());
        EXPECT_TRUE(tree.is_empty());
    }

    // AVL tree balance test
    TEST_F(avl_tree_test, height_test) {
        std::vector<constructor_stub> stubs = get_random_number_vector(LIMIT);
        stub_tree_type tree(stubs.cbegin(), stubs.cend());
        EXPECT_LE(compute_height(tree.get_sentinel() -> left_child.get()), height_limit(tree.size()));
    }

    // Mixed stress tests
    TEST_F(avl_tree_test, primitive_mixed_stress_test) {
        int_tree_type tree;
        std::map<int, constructor_stub> stub_map;
        for (int i = 0; i < SRTESS_LIMIT; i++) {
            int num = random_number();
            switch (do_action_lottery()) {
                case LOOK_UP: {
                    EXPECT_EQ(stub_map.find(num) == stub_map.cend(), tree.find(num) == tree.end());
                    break;
                }
                case INSERT: {
                    auto expect_res = stub_map.insert(std::make_pair(num, constructor_stub(num)));
                    auto actual_res = tree.insert(num);
                    EXPECT_EQ(actual_res.first == tree.end(), expect_res.first == stub_map.cend());
                    EXPECT_EQ(actual_res.second, expect_res.second);
                    break;
                }
                case DELETE: {
                    EXPECT_EQ(stub_map.erase(num), tree.erase(num));
                    break;
                }
            }
        }
    }

    TEST_F(avl_tree_test, mixed_stress_test) {
        stub_tree_type tree;
        std::set<constructor_stub, constructor_stub_comparator> stub_set;
        for (int i = 0; i < SRTESS_LIMIT; i++) {
            int num = random_number();
            switch (do_action_lottery()) {
                case LOOK_UP: {
                    EXPECT_EQ(stub_set.find(num) == stub_set.cend(), tree.find(num) == tree.end());
                    break;
                }
                case INSERT: {
                    auto expect_res = stub_set.insert(num);
                    auto actual_res = tree.insert(num);
                    EXPECT_EQ(actual_res.first == tree.end(), expect_res.first == stub_set.cend());
                    EXPECT_EQ(actual_res.second, expect_res.second);
                    break;
                }
                case DELETE: {
                    EXPECT_EQ(stub_set.erase(num), tree.erase(num));
                    break;
                }
            }
        }
    }
}
