#include <benchmark/benchmark.h>
#include "src/vector/vector.h"
#include <vector>
#include "utility/elements.h"
#include "utility/utility.h"

namespace {
    using namespace algo;
    constexpr static int prime = 97;
    constexpr static int insertion_small_ratio = 100;
    constexpr static int insertion_big_ratio = 5;
    constexpr static int char_range_begin = 1<<12;
    constexpr static int char_range_end = 1<<19;
    constexpr static int small_range_begin = 1<<10;
    constexpr static int small_range_end = 1<<17;
    constexpr static int medium_range_begin = 1<<8;
    constexpr static int medium_range_end = 1<<15;
    constexpr static int big_range_begin = 1<<5;
    constexpr static int big_range_end = 1<<12;

    #define BENCH_SINGLE(name, bigo) \
        BENCHMARK(name<vector<small_element>>)->Name("algo/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::vector<small_element>>)->Name("std/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo)

    #define BENCH_ALL(name, bigo) \
        BENCHMARK(name<vector<char>>)->Name("algo/" #name "/char")->RangeMultiplier(2)->Range(char_range_begin, char_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::vector<char>>)->Name("std/" #name "/char")->RangeMultiplier(2)->Range(char_range_begin, char_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<vector<small_element>>)->Name("algo/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::vector<small_element>>)->Name("std/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<vector<medium_element>>)->Name("algo/" #name "/medium_element")->RangeMultiplier(2)->Range(medium_range_begin, medium_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::vector<medium_element>>)->Name("std/" #name "/medium_element")->RangeMultiplier(2)->Range(medium_range_begin, medium_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<vector<big_element>>)->Name("algo/" #name "/big_element")->RangeMultiplier(2)->Range(big_range_begin, big_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::vector<big_element>>)->Name("std/" #name "/big_element")->RangeMultiplier(2)->Range(big_range_begin, big_range_end)->Complexity(benchmark::bigo)


    template<typename Vector>
    static void vector_begin(benchmark::State& state) {
        Vector vec(state.range(0));
        for (auto _ : state) {
            auto it = vec.begin();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_end(benchmark::State& state) {
        Vector vec(state.range(0));
        for (auto _ : state) {
            auto it = vec.end();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_empty(benchmark::State& state) {
        Vector vec(state.range(0));
        for (auto _ : state) {
            bool is_empty = vec.empty();
            benchmark::DoNotOptimize(is_empty);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_size(benchmark::State& state) {
        Vector vec(state.range(0));
        for (auto _ : state) {
            std::size_t size = vec.size();
            benchmark::DoNotOptimize(size);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_iterate(benchmark::State& state) {
        Vector vec(state.range(0));
        for (auto _ : state) {
            for (auto& element : vec) {
                benchmark::DoNotOptimize(element);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_for_loop(benchmark::State& state) {
        Vector vec(state.range(0));
        for (auto _ : state) {
            for (std::size_t i = 0; i < vec.size(); ++i) {
                benchmark::DoNotOptimize(vec[i]);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_sort(benchmark::State& state) {
        for (auto _ : state) {
            state.PauseTiming();
            Vector vec(state.range(0));
            for (long long i = 0, pos = 0; i < state.range(0); ++i, pos = (pos + prime) % vec.size()) {
                vec[pos] = i;
            }
            state.ResumeTiming();
            std::sort(vec.begin(), vec.end());
            cleanup(state, vec);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_reverse_sort(benchmark::State& state) {
        for (auto _ : state) {
            state.PauseTiming();
            Vector vec(state.range(0));
            for (long long i = 0, pos = 0; i < state.range(0); ++i, pos = (pos + prime) % vec.size()) {
                vec[pos] = i;
            }
            state.ResumeTiming();
            std::sort(vec.rbegin(), vec.rend());
            cleanup(state, vec);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_push_back(benchmark::State& state) {
        Vector vec;
        typename Vector::value_type element{};
        for (auto _ : state) {
            for (int i = 0; i < state.range(0); ++i) {
                vec.push_back(element);
            }
            state.PauseTiming();
            vec.clear();
            state.ResumeTiming();
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_pop_back(benchmark::State& state) {
        for (auto _ : state) {
            state.PauseTiming();
            Vector vec(state.range(0));
            state.ResumeTiming();
            // Use volatile to prevent the loop to be optimized
            volatile unsigned long size = 0;
            while (vec.size() > size) {
                vec.pop_back();
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_insert_single(benchmark::State& state) {
        typename Vector::value_type element{};
        Vector vec(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            vec.pop_back();
            state.ResumeTiming();
            vec.insert(vec.begin() + vec.size() / 2, element);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Vector>
    static void vector_erase_single(benchmark::State& state) {
        typename Vector::value_type element{};
        Vector vec(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            vec.push_back(element);
            state.ResumeTiming();
            vec.erase(vec.begin() + vec.size() / 2);
        }
        state.SetComplexityN(state.range(0));
    }

    BENCH_SINGLE(vector_begin, o1);
    BENCH_SINGLE(vector_end, o1);
    BENCH_SINGLE(vector_empty, o1);
    BENCH_SINGLE(vector_size, o1);
    BENCH_ALL(vector_iterate, oN);
    BENCH_ALL(vector_for_loop, oN);
    BENCH_SINGLE(vector_sort, oNLogN);
    BENCH_SINGLE(vector_reverse_sort, oNLogN);
    BENCH_ALL(vector_push_back, oN);
    BENCH_ALL(vector_pop_back, oN);
    BENCH_ALL(vector_insert_single, oN);
    BENCH_ALL(vector_erase_single, oN);
}