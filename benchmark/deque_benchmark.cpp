#include <benchmark/benchmark.h>
#include "src/deque/deque.h"
#include <deque>
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
        BENCHMARK(name<deque<small_element>>)->Name("algo/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::deque<small_element>>)->Name("std/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo)

    #define BENCH_ALL(name, bigo) \
        BENCHMARK(name<deque<char>>)->Name("algo/" #name "/char")->RangeMultiplier(2)->Range(char_range_begin, char_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::deque<char>>)->Name("std/" #name "/char")->RangeMultiplier(2)->Range(char_range_begin, char_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<deque<small_element>>)->Name("algo/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::deque<small_element>>)->Name("std/" #name "/small_element")->RangeMultiplier(2)->Range(small_range_begin, small_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<deque<medium_element>>)->Name("algo/" #name "/medium_element")->RangeMultiplier(2)->Range(medium_range_begin, medium_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::deque<medium_element>>)->Name("std/" #name "/medium_element")->RangeMultiplier(2)->Range(medium_range_begin, medium_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<deque<big_element>>)->Name("algo/" #name "/big_element")->RangeMultiplier(2)->Range(big_range_begin, big_range_end)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::deque<big_element>>)->Name("std/" #name "/big_element")->RangeMultiplier(2)->Range(big_range_begin, big_range_end)->Complexity(benchmark::bigo)

    template<typename Deque>
    static void deque_begin(benchmark::State& state) {
        Deque deq(state.range(0));
        for (auto _ : state) {
            auto it = deq.begin();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_end(benchmark::State& state) {
        Deque deq(state.range(0));
        for (auto _ : state) {
            auto it = deq.end();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_empty(benchmark::State& state) {
        Deque deq(state.range(0));
        for (auto _ : state) {
            bool is_empty = deq.empty();
            benchmark::DoNotOptimize(is_empty);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_size(benchmark::State& state) {
        Deque deq(state.range(0));
        for (auto _ : state) {
            std::size_t size = deq.size();
            benchmark::DoNotOptimize(size);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_iterate(benchmark::State& state) {
        Deque deq(state.range(0));
        typename Deque::value_type acc{};
        for (auto _ : state) {
            for (auto& element : deq) {
                acc += element;
            }
            benchmark::DoNotOptimize(acc);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_for_loop(benchmark::State& state) {
        Deque deq(state.range(0));
        typename Deque::value_type acc{};
        for (auto _ : state) {
            for (std::size_t i = 0; i < deq.size(); ++i) {
                acc += deq[i];
            }
            benchmark::DoNotOptimize(acc);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_sort(benchmark::State& state) {
        for (auto _ : state) {
            state.PauseTiming();
            Deque deq(state.range(0));
            for (long long i = 0, pos = 0; i < state.range(0); ++i, pos = (pos + prime) % deq.size()) {
                deq[pos] = i;
            }
            state.ResumeTiming();
            std::sort(deq.begin(), deq.end());
            cleanup(state, deq);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_reverse_sort(benchmark::State& state) {
        for (auto _ : state) {
            state.PauseTiming();
            Deque deq(state.range(0));
            for (long long i = 0, pos = 0; i < state.range(0); ++i, pos = (pos + prime) % deq.size()) {
                deq[pos] = i;
            }
            state.ResumeTiming();
            std::sort(deq.rbegin(), deq.rend());
            cleanup(state, deq);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_push_back(benchmark::State& state) {
        Deque deq;
        typename Deque::value_type element{};
        for (auto _ : state) {
            for (int i = 0; i < state.range(0); ++i) {
                ++element;
                deq.push_back(element);
            }
            state.PauseTiming();
            deq.clear();
            state.ResumeTiming();
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_pop_back(benchmark::State& state) {
        Deque deq;
        for (auto _ : state) {
            state.PauseTiming();
            deq.resize(state.range(0));
            state.ResumeTiming();
            while (!deq.empty()) {
                deq.pop_back();
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_push_front(benchmark::State& state) {
        Deque deq;
        typename Deque::value_type element{};
        for (auto _ : state) {
            for (int i = 0; i < state.range(0); ++i) {
                ++element;
                deq.push_front(element);
            }
            state.PauseTiming();
            deq.clear();
            state.ResumeTiming();
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_pop_front(benchmark::State& state) {
        Deque deq;
        for (auto _ : state) {
            state.PauseTiming();
            deq.resize(state.range(0));
            benchmark::DoNotOptimize(deq);
            state.ResumeTiming();
            for (int i = 1; i < state.range(0); ++i) {
                deq.pop_front();
            }
            benchmark::DoNotOptimize(deq.front());
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_shift_left(benchmark::State& state) {
        Deque deq;
        typename Deque::value_type element{};
        for (auto _ : state) {
            for (int i = 0; i < state.range(0); ++i) {
                deq.push_front(element);
                deq.pop_back();
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_shift_right(benchmark::State& state) {
        Deque deq;
        typename Deque::value_type element{};
        for (auto _ : state) {
            for (int i = 0; i < state.range(0); ++i) {
                deq.push_back(element);
                deq.pop_front();
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Deque>
    static void deque_insert_single_left(benchmark::State& state) {
        typename Deque::value_type element{};
        Deque deq(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            deq.pop_front();
            state.ResumeTiming();
            deq.insert(deq.begin() + deq.size() / 3, element);
        }
        state.SetComplexityN(state.range(0) / 3);
    }

    template<typename Deque>
    static void deque_insert_single_right(benchmark::State& state) {
        typename Deque::value_type element{};
        Deque deq(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            deq.pop_back();
            state.ResumeTiming();
            deq.insert(deq.begin() + deq.size() * 2 / 3, element);
        }
        state.SetComplexityN(state.range(0) / 3);
    }

    template<typename Deque>
    static void deque_erase_single_left(benchmark::State& state) {
        typename Deque::value_type element{};
        Deque deq(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            deq.push_front(element);
            state.ResumeTiming();
            deq.erase(deq.begin() + deq.size() / 3);
        }
        state.SetComplexityN(state.range(0) / 3);
    }

    template<typename Deque>
    static void deque_erase_single_right(benchmark::State& state) {
        typename Deque::value_type element{};
        Deque deq(state.range(0));
        for (auto _ : state) {
            state.PauseTiming();
            deq.push_back(element);
            state.ResumeTiming();
            deq.erase(deq.begin() + deq.size() * 2 / 3);
        }
        state.SetComplexityN(state.range(0) / 3);
    }

    BENCH_SINGLE(deque_begin, o1);
    BENCH_SINGLE(deque_end, o1);
    BENCH_SINGLE(deque_empty, o1);
    BENCH_SINGLE(deque_size, o1);
    BENCH_ALL(deque_iterate, oN);
    BENCH_ALL(deque_for_loop, oN);
    BENCH_SINGLE(deque_sort, oNLogN);
    BENCH_SINGLE(deque_reverse_sort, oNLogN);
    BENCH_ALL(deque_push_back, oN);
    BENCH_ALL(deque_pop_back, oN);
    BENCH_ALL(deque_push_front, oN);
    BENCH_ALL(deque_pop_front, oN);
    BENCH_ALL(deque_shift_left, oN);
    BENCH_ALL(deque_shift_right, oN);
    BENCH_ALL(deque_insert_single_left, oN);
    BENCH_ALL(deque_insert_single_right, oN);
    BENCH_ALL(deque_erase_single_left, oN);
    BENCH_ALL(deque_erase_single_right, oN);
}