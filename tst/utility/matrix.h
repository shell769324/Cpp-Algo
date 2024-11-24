#pragma once
#include <vector>
#include <random>

namespace algo {
struct matrix_type {
    matrix_type();
    matrix_type(const std::vector<std::vector<long double> >& matrix);
    std::size_t size() const noexcept;
    std::vector<std::vector<long double> > matrix;
    friend bool operator==(const matrix_type& matrix1, const matrix_type& matrix2) noexcept;
};

bool is_square_matrix_equal(const matrix_type& mat1, const matrix_type& mat2) noexcept;
matrix_type get_random_rotation_matrix();
matrix_type square_matrix_multiply(const matrix_type& A, const matrix_type& B);

struct square_matrix_mult {
    matrix_type operator()(const matrix_type& A, const matrix_type& B) const noexcept;
};

struct square_matrix_multiply_inverse {
    matrix_type operator()(const matrix_type& operand, const matrix_type& product) const noexcept;
};
}