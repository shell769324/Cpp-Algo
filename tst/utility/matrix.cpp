#include "matrix.h"

namespace algo {
bool is_square_matrix_equal(const matrix_type& mat1, const matrix_type& mat2) noexcept {
    if (mat1.size() != mat2.size()) {
        return false;
    }
    for (std::size_t i = 0; i < mat1.size(); ++i) {
        for (std::size_t j = 0; j < mat1.matrix[0].size(); ++j) {
            if (abs(mat2.matrix[i][j] - mat1.matrix[i][j]) / std::min(abs(mat1.matrix[i][j]), abs(mat2.matrix[i][j])) >= 1e-3 &&
                abs(mat2.matrix[i][j] - mat1.matrix[i][j]) > 1e-5) {
                return false;
            }
        }
    }
    return true;
}

std::default_random_engine generator(17);
std::uniform_real_distribution<double> pi_distribution(0, 3.141592);

matrix_type::matrix_type() : matrix({{1.0, 0}, {0, 1.0}}) { }

matrix_type::matrix_type(const std::vector<std::vector<long double> >& matrix) : matrix(matrix) { }

std::size_t matrix_type::size() const noexcept { return matrix.size(); }

bool operator==(const matrix_type& matrix1, const matrix_type& matrix2) noexcept {
    return is_square_matrix_equal(matrix1, matrix2);
}

matrix_type get_random_rotation_matrix() {
    long double angle = pi_distribution(generator);
    long double cos = std::cos(angle);
    long double sin = std::pow(1 - cos * cos, 0.5);
    return matrix_type({{cos, -sin}, {sin, cos}});
}

matrix_type square_matrix_multiply(const matrix_type& A, const matrix_type& B) {
    std::vector<std::vector<long double> > res(A.size(), std::vector<long double>(B.matrix[0].size(), 0));
    for (std::size_t i = 0; i < A.size(); ++i) {
        for (std::size_t j = 0; j < A.matrix[0].size(); ++j) {
            for (std::size_t k = 0; k < A.matrix[0].size(); ++k) {
                res[i][j] += A.matrix[i][k] * B.matrix[k][j];
            }
        }
    }
    return res;
};


matrix_type square_matrix_mult::operator()(const matrix_type& A, const matrix_type& B) const noexcept { 
    return square_matrix_multiply(A, B);
};

matrix_type square_matrix_multiply_inverse::operator()(const matrix_type& operand, const matrix_type& product) const noexcept { 
    long double determinant = operand.matrix[0][0] * operand.matrix[1][1] - operand.matrix[0][1] * operand.matrix[1][0];
    matrix_type inverse({{operand.matrix[1][1], -operand.matrix[0][1]}, {-operand.matrix[1][0], operand.matrix[0][0]}});
    for (auto& vec : inverse.matrix) {
        for (auto& num : vec) {
            num /= determinant;
        }
    }
    return square_matrix_mult()(inverse, product);
}
}