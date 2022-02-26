#include <concepts>
#include <vector>

namespace algo {
template<typename T>
double expectation(std::vector<T>& vec) requires std::is_arithmetic_v<T> {
    return (double) std::accumulate(vec.begin(), vec.end(), 0.0, std::plus<T>()) / vec.size();
}

template<typename T>
double stdev(std::vector<T>& vec) requires std::is_arithmetic_v<T> {
    std::vector<double> squared;
    squared.resize(vec.size());
    std::transform(vec.begin(), vec.end(), squared.begin(), [](T d) { return d * d; } );
    double average = expectation(vec);
    double variance = expectation(squared) - average * average;
    return std::pow(variance, 0.5);
}

template<typename T, typename U>
double covariance(std::vector<T>& vec1, std::vector<U>& vec2) requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)  {
    std::vector<double> pointwise_products;
    pointwise_products.resize(vec1.size());
    std::transform(vec1.begin(), vec1.end(), vec2.begin(), pointwise_products.begin(), [](T a, U b) { return (double) a * b; } );
    return expectation(pointwise_products) - expectation(vec1) * expectation(vec2);
}

template<typename T, typename U>
double correlation(std::vector<T>& vec1, std::vector<U>& vec2) requires (std::is_arithmetic_v<T> && std::is_arithmetic_v<U>) {
    return covariance(vec1, vec2) / (stdev(vec1) * stdev(vec2));
}
}