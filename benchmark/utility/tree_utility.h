#pragma once
#include <benchmark/benchmark.h>
#include <array>
#include <cmath>
#include "elements.h"

namespace algo {
    // It is important to build trees through randomized insertion orders since different insertion orders
    // yield different trees.
    constexpr static std::array<int, 18> primes = {100003, 182779, 239461, 299281, 355483, 389437,
                                               420263, 472561, 543773, 597269, 663407, 685613,
                                               706267, 744761, 809903, 857287, 903029, 966907};
    constexpr static int scaler = 8;
    constexpr static int step = 12;
    constexpr static int set_operation_iter = 25;

    template<typename T>
    constexpr std::size_t range_begin = 0;

    template<typename T>
    constexpr std::size_t range_end = 0;

    template<>
    constexpr inline std::size_t range_begin<int> = 1<<10;

    template<>
    constexpr inline std::size_t range_begin<small_element> = 1<<9;

    template<>
    constexpr inline std::size_t range_begin<medium_element> = 1<<7;

    template<>
    constexpr inline std::size_t range_begin<big_element> = 1<<5;

    template<>
    constexpr inline std::size_t range_end<int> = 1<<17;

    template<>
    constexpr inline std::size_t range_end<small_element> = 1<<16;

    template<>
    constexpr inline std::size_t range_end<medium_element> = 1<<14;

    template<>
    constexpr inline std::size_t range_end<big_element> = 1<<12;

    template <typename T>
    static double set_op_complexity(benchmark::IterationCount n) {
        return n * std::log2(range_end<T> / n + 1);
    }

    template <typename T>
    static double std_set_op_complexity(benchmark::IterationCount n) {
        return n * std::log2(range_end<T>);
    }
};
