#include "gtest/gtest.h"
#include "tree/binary_indexed_tree.h"
#include "tst/utility/constructor_stub.h"
#include "tst/utility/tracking_allocator.h"
#include <random>
#include <cmath>

namespace {
    using namespace algo;
    std::default_random_engine generator(17);
    std::uniform_real_distribution<double> pi_distribution(0, 3.141592);
    std::uniform_int_distribution<int> int_distribution(-1000, 1000);
    std::uniform_int_distribution<int> index_distribution(0, 1000000);

    static const int SMALL_LIMIT = 10;
    static const int MEDIUM_LIMIT = 64;
    static const int BIG_LIMIT = 500;
    static const int HUGE_LIMIT = 2000;
    static const auto plus_inverse = [](const std::pair<int, bool>& operand, int sum) { return sum - operand.first; };
    using plus_tree = binary_indexed_tree<int, std::plus<int>, std::remove_cv_t<decltype(plus_inverse)>, tracking_allocator<int> >;
    using matrix_t = std::vector<std::vector<long double> >;


    class binary_indexed_tree_test : public ::testing::Test {
    public:
        tracking_allocator<int> int_allocator;
        tracking_allocator<matrix_t> matrix_allocator;
    protected:
        virtual void SetUp() {
            tracking_allocator<int>::reset();
            tracking_allocator<matrix_t>::reset();
            constructor_stub::reset_constructor_destructor_counter();
        }

        virtual void TearDown() {
            EXPECT_EQ(constructor_stub::constructor_invocation_count, constructor_stub::destructor_invocation_count);
            tracking_allocator<int>::check();
            tracking_allocator<matrix_t>::check();
        }
    };

    static const auto square_matrix_multiply = 
    [](const matrix_t& A, const matrix_t& B) {
        matrix_t res(A.size(), std::vector<long double>(B[0].size(), 0));
        for (std::size_t i = 0; i < A.size(); ++i) {
            for (std::size_t j = 0; j < A[0].size(); ++j) {
                for (std::size_t k = 0; k < A[0].size(); ++k) {
                    res[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return res;
    };

    static const auto square_matrix_multiply_inverse = [](const std::pair<matrix_t, bool>& operand, const matrix_t& product) {
        long double determinant = operand.first[0][0] * operand.first[1][1] - operand.first[0][1] * operand.first[1][0];
        matrix_t inverse = {{operand.first[1][1], -operand.first[0][1]}, {-operand.first[1][0], operand.first[0][0]}};
        for (auto& vec : inverse) {
            for (auto& num : vec) {
                num /= determinant;
            }
        }
        if (operand.second) {
            return square_matrix_multiply(inverse, product);
        }
        return square_matrix_multiply(product, inverse);
    };

    static const matrix_t identity_matrix = {{1.0, 0}, {0, 1.0}};
    static const matrix_t mat1 = {{0.8, 1.6}, {1, 0.5}};
    static const matrix_t mat2 = {{1.2, 0.7}, {1.3, 0.3}};
    using matrix_mult_tree = binary_indexed_tree<matrix_t, std::remove_cv_t<decltype(square_matrix_multiply)>, 
            std::remove_cv_t<decltype(square_matrix_multiply_inverse)>, tracking_allocator<matrix_t> >;

    bool is_square_matrix_equal(const matrix_t& mat1, const matrix_t& mat2) {
        if (mat1.size() != mat2.size()) {
            return false;
        }
        for (std::size_t i = 0; i < mat1.size(); ++i) {
            for (std::size_t j = 0; j < mat1[0].size(); ++j) {
                if (abs(mat2[i][j] - mat1[i][j]) / std::min(abs(mat1[i][j]), abs(mat2[i][j])) >= 1e-3 &&
                    abs(mat2[i][j] - mat1[i][j]) > 1e-5) {
                    return false;
                }
            }
        }
        return true;
    }

    matrix_t getRandomRotationMatrix() {
        long double angle = pi_distribution(generator);
        long double cos = std::cos(angle);
        long double sin = std::pow(1 - cos * cos, 0.5);
        return {{cos, -sin}, {sin, cos}};
    }

    TEST_F(binary_indexed_tree_test, matrix_mult_test) {
        auto mat = square_matrix_multiply(mat1, mat2);
        auto mat2_back = square_matrix_multiply_inverse(make_pair(mat1, true), mat);
        auto mat1_back = square_matrix_multiply_inverse(make_pair(mat2, false), mat);
        EXPECT_TRUE(is_square_matrix_equal(mat1_back, mat1));
        EXPECT_TRUE(is_square_matrix_equal(mat2_back, mat2));
    }

    TEST_F(binary_indexed_tree_test, default_fill_constructor_test) {
        plus_tree tree(SMALL_LIMIT, std::plus<int>(), plus_inverse, 0, this -> int_allocator);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), 0);
            }
        }
        EXPECT_EQ(tree.get_allocator(), this -> int_allocator);
        EXPECT_EQ(tree.get_identity(), 0);
    }

    TEST_F(binary_indexed_tree_test, fill_constructor_test) {
        plus_tree tree(SMALL_LIMIT, 1, std::plus<int>(), plus_inverse, 0, this -> int_allocator);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), j - i);
            }
        }
        EXPECT_EQ(tree.get_allocator(), this -> int_allocator);
        EXPECT_EQ(tree.get_identity(), 0);
    }

    TEST_F(binary_indexed_tree_test, range_constructor_test) {
        std::vector<int> nums(SMALL_LIMIT);
        for (int& num : nums) {
            num = int_distribution(generator);
        }
        plus_tree tree(nums.begin(), nums.end(), std::plus<int>(), plus_inverse, 0, this -> int_allocator);
        EXPECT_EQ(tree.size(), nums.size());
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_EQ(tree.query(i, j), std::accumulate(nums.begin() + i, nums.begin() + j, 0));
            }
        }
        EXPECT_EQ(tree.get_allocator(), this -> int_allocator);
    }

    TEST_F(binary_indexed_tree_test, copy_constructor_test) {
        plus_tree tree(SMALL_LIMIT, 1, std::plus<int>(), plus_inverse);
        EXPECT_EQ(tree.get_identity(), 0);
        plus_tree tree_copy(tree);
        for (std::size_t i = 0; i < tree_copy.size(); ++i) {
            for (std::size_t j = i; j < tree_copy.size(); ++j) {
                EXPECT_EQ(tree_copy.query(i, j), j - i);
            }
        }
        EXPECT_EQ(tree.get_allocator(), tree_copy.get_allocator());
    }

    TEST_F(binary_indexed_tree_test, move_constructor_test) {
        plus_tree tree(SMALL_LIMIT, 1, std::plus<int>(), plus_inverse, 0, this -> int_allocator);
        plus_tree tree_copy(std::move(tree));
        for (std::size_t i = 0; i < tree_copy.size(); ++i) {
            for (std::size_t j = i; j < tree_copy.size(); ++j) {
                EXPECT_EQ(tree_copy.query(i, j), j - i);
            }
        }
        EXPECT_EQ(tree.size(), 0);
        EXPECT_EQ(tree_copy.get_allocator(), this -> int_allocator);
    }

    TEST_F(binary_indexed_tree_test, query_basic_test) {
        matrix_mult_tree tree(SMALL_LIMIT, square_matrix_multiply, square_matrix_multiply_inverse, identity_matrix);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), identity_matrix));
            }
        }
    }

    TEST_F(binary_indexed_tree_test, update_basic_test) {
        matrix_mult_tree tree(SMALL_LIMIT, square_matrix_multiply, square_matrix_multiply_inverse, identity_matrix);
        std::size_t pos = SMALL_LIMIT / 2;
        tree.update(pos, mat1);
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                if (i <= pos && pos < j) {
                    EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), mat1));
                } else {
                    EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), identity_matrix));
                }
            }
        }
    }

    TEST_F(binary_indexed_tree_test, update_intermediate_test) {
        matrix_mult_tree tree(SMALL_LIMIT, square_matrix_multiply, square_matrix_multiply_inverse, identity_matrix);
        std::vector<matrix_t> mats;
        for (std::size_t i = 0; i < SMALL_LIMIT; ++i) {
            if (i % 3 == 0) {
                mats.push_back(identity_matrix);
            } else if (i % 3 == 1) {
                mats.push_back(mat1);
                tree.update(i, mat1);
            } else {
                mats.push_back(mat2);
                tree.update(i, mat2);
            }
        }
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                matrix_t expected(identity_matrix);
                for (std::size_t k = i; k < j; ++k) {
                    expected = square_matrix_multiply(expected, mats[k]);
                }
                EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), expected));
            }
        }
    }

    TEST_F(binary_indexed_tree_test, update_stress_matrix_mult_test) {
        matrix_mult_tree tree(MEDIUM_LIMIT, square_matrix_multiply, square_matrix_multiply_inverse, identity_matrix);
        std::vector<matrix_t> mats(MEDIUM_LIMIT);
        for (std::size_t i = 0; i < MEDIUM_LIMIT; ++i) {
            matrix_t mat = getRandomRotationMatrix();
            mats[i] = mat;
            tree.update(i, mat);
        }
        int jump = 3;
        for (std::size_t i = 0; i < tree.size(); i += jump) {
            for (std::size_t j = i; j < tree.size(); j += jump) {
                matrix_t expected(identity_matrix);
                for (std::size_t k = i; k < j; ++k) {
                    expected = square_matrix_multiply(expected, mats[k]);
                }
                EXPECT_TRUE(is_square_matrix_equal(tree.query(i, j), expected));
            }
        }
    }

    TEST_F(binary_indexed_tree_test, update_stress_addition_test) {
        plus_tree tree(BIG_LIMIT, std::plus<int>(), plus_inverse);
        std::vector<int> nums(BIG_LIMIT);
        for (std::size_t i = 0; i < BIG_LIMIT; ++i) {
            nums[i] = int_distribution(generator);
            tree.update(i, nums[i]);
        }
        std::vector<int> prefix_sum(BIG_LIMIT);
        std::partial_sum(nums.begin(), nums.end(), prefix_sum.begin());
        for (std::size_t i = 0; i < tree.size(); ++i) {
            for (std::size_t j = i; j < tree.size(); ++j) {
                int sum = i == j ? 0 : (prefix_sum[j - 1] - (i == 0 ? 0 : prefix_sum[i - 1]));
                EXPECT_EQ(tree.query(i, j), sum);
            }
        }
    }

    TEST_F(binary_indexed_tree_test, mixed_matrix_mult_test) {
        matrix_mult_tree tree(MEDIUM_LIMIT, square_matrix_multiply, square_matrix_multiply_inverse, identity_matrix);
        std::vector<matrix_t> mats(tree.size(), identity_matrix);
        for (std::size_t i = 0; i < HUGE_LIMIT; ++i) {
            int lottery = int_distribution(generator) % 2;
            if (lottery) {
                // update
                int idx = index_distribution(generator) % mats.size();
                matrix_t mat = getRandomRotationMatrix();
                mats[idx] = mat;
                tree.update(idx, mat);
            } else {
                // query
                int idx1 = index_distribution(generator) % mats.size();
                int idx2 = index_distribution(generator) % mats.size();
                if (idx1 > idx2) {
                    std::swap(idx1, idx2);
                }
                matrix_t expected(identity_matrix);
                for (int j = idx1; j < idx2; ++j) {
                    expected = square_matrix_multiply(expected, mats[j]);
                }
                EXPECT_TRUE(is_square_matrix_equal(tree.query(idx1, idx2), expected));
            }
        }
    }

    TEST_F(binary_indexed_tree_test, mixed_addition_test) {
        plus_tree tree(HUGE_LIMIT, std::plus<int>(), plus_inverse);
        std::vector<int> nums(tree.size());
        for (std::size_t i = 0; i < HUGE_LIMIT; ++i) {
            int lottery = int_distribution(generator) % 2;
            if (lottery) {
                // update
                int idx = index_distribution(generator) % nums.size();
                int num = int_distribution(generator);
                nums[idx] = num;
                tree.update(idx, num);
            } else {
                // query
                int idx1 = index_distribution(generator) % nums.size();
                int idx2 = index_distribution(generator) % nums.size();
                if (idx1 > idx2) {
                    std::swap(idx1, idx2);
                }
                EXPECT_EQ(tree.query(idx1, idx2), std::accumulate(nums.begin() + idx1, nums.begin() + idx2, 0));
            }
        }
    }
}