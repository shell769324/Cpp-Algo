#include <benchmark/benchmark.h>
#include "src/range_query_tree/binary_indexed_tree.h"
#include "src/range_query_tree/range_segment_tree.h"
#include "src/range_query_tree/segment_tree.h"
#include "utility/tree_utility.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

namespace {
    using namespace algo;

    constexpr static int prime1 = primes[0];
    constexpr static int prime2 = primes[1];

    struct plus_inverse {
        int operator()(const int& operand, const int& sum) const noexcept {
            return sum - operand;
        };
    };

    struct int_max {
        int operator()(const int& num1, const int& num2) const noexcept {
            return std::max(num1, num2);
        };
    };

    struct int_max_repeat {
        int operator()(std::size_t, const int& num) const noexcept {
            return num;
        }
    };

    #define BENCH_PLUS(name, bigo) \
        BENCHMARK(name<binary_indexed_tree<int, std::plus<int>, plus_inverse>>)->Name("binary_indexed_tree/" #name "/plus")->RangeMultiplier(2)->Range(range_begin<int>, range_end<int>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<segment_tree<int, std::plus<int>>>)->Name("segment_tree/" #name "/plus")->RangeMultiplier(2)->Range(range_begin<int>, range_end<int>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<range_segment_tree<int, std::plus<int>, std::multiplies<int>>>)->Name("range_segment_tree/" #name "/plus")->RangeMultiplier(2)->Range(range_begin<int>, range_end<int>)->Complexity(benchmark::bigo)

    #define BENCH_MAX(name, bigo) \
        BENCHMARK(name<segment_tree<int, int_max>>)->Name("segment_tree/" #name "/max")->RangeMultiplier(2)->Range(range_begin<int>, range_end<int>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<range_segment_tree<int, int_max, int_max_repeat>>)->Name("range_segment_tree/" #name "/max")->RangeMultiplier(2)->Range(range_begin<int>, range_end<int>)->Complexity(benchmark::bigo)

    template<typename Tree>
    static void range_query_query(benchmark::State& state) {
        std::vector<int> source(state.range(0));
        std::iota(source.begin(), source.end(), 0);
        Tree tree(source.begin(), source.end());
        std::vector<int> starts(state.range(0));
        std::vector<int> ends(state.range(0));
        for (std::size_t i = 0; i < starts.size(); ++i) {
            starts[i] = prime1 * i % source.size();
            ends[i] = 1 + prime2 * i % (source.size() - starts[i]) + starts[i];
        }
        for (auto _ : state) {
            for (std::size_t i = 0; i < starts.size(); ++i) {
                int result = tree.query(starts[i], ends[i]);
                benchmark::DoNotOptimize(result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Tree>
    static void range_query_update(benchmark::State& state) {
        std::vector<int> source(state.range(0));
        std::iota(source.begin(), source.end(), 0);
        Tree tree(source.begin(), source.end());
        std::vector<int> positions(state.range(0));
        std::vector<int> values(state.range(0));
        for (std::size_t i = 0; i < positions.size(); ++i) {
            positions[i] = prime1 * i % source.size();
            values[i] = prime2 * i % source.size();
        }
        for (auto _ : state) {
            for (std::size_t i = 0; i < positions.size(); ++i) {
                tree.update(positions[i], values[i]);
            }
            benchmark::DoNotOptimize(tree);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Tree>
    static void range_query_prefix_search(benchmark::State& state) {
        std::vector<int> source(state.range(0));
        std::iota(source.begin(), source.end(), 0);
        std::vector<int> starts(state.range(0));
        std::vector<int> ends(state.range(0));
        for (std::size_t i = 0; i < starts.size(); ++i) {
            starts[i] = prime1 * i % source.size();
            ends[i] = 1 + prime2 * i % (source.size() - starts[i]) + starts[i];
        }
        auto more_than = [threshold = state.range(0) / 2](const int& num) {
            return num > threshold;
        };
        std::random_device device;
        std::mt19937 gen(device());
        for (auto _ : state) {
            state.PauseTiming();
            std::shuffle(source.begin(), source.end(), gen);
            Tree tree(source.begin(), source.end());
            state.ResumeTiming();
            for (std::size_t i = 0; i < starts.size(); ++i) {
                std::optional<int> result = tree.prefix_search(more_than, starts[i], ends[i]);
                benchmark::DoNotOptimize(result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Tree>
    static void range_query_suffix_search(benchmark::State& state) {
        std::vector<int> source(state.range(0));
        std::iota(source.begin(), source.end(), 0);
        std::vector<int> starts(state.range(0));
        std::vector<int> ends(state.range(0));
        for (std::size_t i = 0; i < starts.size(); ++i) {
            starts[i] = prime1 * i % source.size();
            ends[i] = 1 + prime2 * i % (source.size() - starts[i]) + starts[i];
        }
        auto more_than = [threshold = state.range(0) / 2](const int& num) {
            return num > threshold;
        };
        std::random_device device;
        std::mt19937 gen(device());
        for (auto _ : state) {
            state.PauseTiming();
            std::shuffle(source.begin(), source.end(), gen);
            Tree tree(source.begin(), source.end());
            state.ResumeTiming();
            for (std::size_t i = 0; i < starts.size(); ++i) {
                std::optional<int> result = tree.suffix_search(more_than, starts[i], ends[i]);
                benchmark::DoNotOptimize(result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    static void range_query_range_update(benchmark::State& state) {
        std::vector<int> source(state.range(0));
        std::iota(source.begin(), source.end(), 0);
        range_segment_tree<int, int_max, int_max_repeat> tree(source.begin(), source.end());
        std::vector<int> starts(state.range(0));
        std::vector<int> ends(state.range(0));
        std::vector<int> values(state.range(0));
        for (std::size_t i = 0; i < starts.size(); ++i) {
            starts[i] = prime1 * i % source.size();
            ends[i] = 1 + prime2 * i % (source.size() - starts[i]) + starts[i];
            values[i] = prime2 * i % source.size();
        }
        for (auto _ : state) {
            for (std::size_t i = 0; i < starts.size(); ++i) {
                tree.update(starts[i], ends[i], values[i]);
            }
            benchmark::DoNotOptimize(tree);
        }
        state.SetComplexityN(state.range(0));
    }

    BENCH_PLUS(range_query_query, oNLogN);
    BENCH_PLUS(range_query_update, oNLogN);
    BENCH_MAX(range_query_query, oNLogN);
    BENCH_MAX(range_query_update, oNLogN);
    BENCH_MAX(range_query_prefix_search, oNLogN);
    BENCH_MAX(range_query_suffix_search, oNLogN);
    BENCHMARK(range_query_range_update)->Name("range_segment_tree/range_query_range_update/max")->RangeMultiplier(2)->Range(range_begin<int>, range_end<int>)->Complexity(benchmark::oNLogN);
}