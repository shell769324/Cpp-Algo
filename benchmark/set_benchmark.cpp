#include <benchmark/benchmark.h>
#include "src/tree/avl_tree_set.h"
#include "src/tree/red_black_tree_set.h"
#include <set>
#include "utility/elements.h"
#include "utility/utility.h"
#include "utility/tree_utility.h"
#include <vector>
#include <cmath>

namespace {
    using namespace algo;

    #define BENCH_SINGLE(name, type, bigo) \
        BENCHMARK(name<avl_tree_set<type>>)->Name("avl_tree_set/" #name "/" #type)->RangeMultiplier(2)->Range(range_begin<type>, range_end<type>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<red_black_tree_set<type>>)->Name("red_black_tree_set/" #name "/" #type)->RangeMultiplier(2)->Range(range_begin<type>, range_end<type>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::set<type>>)->Name("std/" #name "/" #type)->RangeMultiplier(2)->Range(range_begin<type>, range_end<type>)->Complexity(benchmark::bigo)

    #define BENCH_ALL(name, bigo) \
        BENCH_SINGLE(name, int, bigo); \
        BENCH_SINGLE(name, small_element, bigo); \
        BENCH_SINGLE(name, medium_element, bigo); \
        BENCH_SINGLE(name, big_element, bigo)

    // Use a fix number of iterations since setup and teardown takes much longer than the measured portion of 
    // the benchmark. Google benchmark sets the number of iterations based on the measured portion, not the entirety
    // of the code in the loop.
    #define BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, type) \
        BENCHMARK(name<avl_tree_set<type>>)->Name("avl_tree_set/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(set_op_complexity<type>); \
        BENCHMARK(name<red_black_tree_set<type>>)->Name("red_black_tree_set/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(set_op_complexity<type>); \
        BENCHMARK(name<std::set<type>>)->Name("std/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(std_set_op_complexity<type>)

    #define BENCH_SET_OPERATION_UNBALANCED(name) \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, int); \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, small_element); \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, medium_element); \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, big_element)

    #define BENCH_SINGLE_SET_OPERATION_BALANCED(name, type) \
        BENCHMARK(name<avl_tree_set<type>>)->Name("avl_tree_set/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(benchmark::oN); \
        BENCHMARK(name<red_black_tree_set<type>>)->Name("red_black_tree_set/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(benchmark::oN); \
        BENCHMARK(name<std::set<type>>)->Name("std/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(benchmark::oNLogN)

    #define BENCH_SET_OPERATION_BALANCED(name) \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, int); \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, small_element); \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, medium_element); \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, big_element)

    template<typename Set>
    static void set_begin(benchmark::State& state) {
        Set set;
        for (int i = 0; i < state.range(0); i++) {
            set.insert(i);
        }
        for (auto _ : state) {
            auto it = set.begin();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_end(benchmark::State& state) {
        Set set;
        for (int i = 0; i < state.range(0); i++) {
            set.insert(i);
        }
        for (auto _ : state) {
            auto it = set.end();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_empty(benchmark::State& state) {
        Set set;
        for (int i = 0; i < state.range(0); i++) {
            set.insert(i);
        }
        for (auto _ : state) {
            bool is_empty = set.empty();
            benchmark::DoNotOptimize(is_empty);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_size(benchmark::State& state) {
        Set set;
        for (int i = 0; i < state.range(0); i++) {
            set.insert(i);
        }
        for (auto _ : state) {
            std::size_t size = set.size();
            benchmark::DoNotOptimize(size);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    Set random_insert(int& prime_index, int limit, int count) {
        Set set;
        // Randomize insertion order
        // Also half elements in the range is missing
        for (int i = 0, val = 0; i < count; ++i, val = (val + primes[prime_index]) % limit) {
            set.insert(val);
        }
        prime_index = (prime_index + 1) % primes.size();
        return set;
    }

    template<typename Set>
    Set prepare_set(int& prime_index, benchmark::State& state) {
        state.PauseTiming();
        int half_range = state.range(0) / 2;
        Set set = random_insert<Set>(prime_index, state.range(0), half_range);
        state.ResumeTiming();
        return set;
    }

    template<typename Set>
    static void set_iterate(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Set set = prepare_set<Set>(prime_index, state);
            typename Set::value_type element{0};
            for (auto& elem : set) {
                element += elem;
            }
            benchmark::DoNotOptimize(element);
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_find(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Set set = prepare_set<Set>(prime_index, state);
            typename Set::value_type element{0};
            for (int i = 0; i < state.range(0); ++i, ++element) {
                auto it = set.find(element);
                benchmark::DoNotOptimize(it);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_lower_bound(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Set set = prepare_set<Set>(prime_index, state);
            typename Set::value_type element{0};
            for (int i = 0; i < state.range(0); ++i, ++element) {
                auto it = set.lower_bound(element);
                benchmark::DoNotOptimize(it);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_upper_bound(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Set set = prepare_set<Set>(prime_index, state);
            typename Set::value_type element{0};
            for (int i = 0; i < state.range(0); ++i, ++element) {
                auto it = set.upper_bound(element);
                benchmark::DoNotOptimize(it);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::vector<T> prepare_vector(int& prime_index, benchmark::State& state) {
        std::vector<T> vec;
        // Randomize insertion order
        for (int i = 0, val = 0; i < state.range(0); ++i, val = (val + primes[prime_index]) % state.range(0)) {
            vec.push_back(val * 2);
        }
        prime_index = (prime_index + 1) % primes.size();
        return vec;
    }

    template<typename Set>
    static void set_insert_absent(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            std::vector<T> vec = prepare_vector<T>(prime_index, state);
            Set set;
            state.ResumeTiming();
            for (auto& elem : vec) {
                set.insert(elem);
            }
            cleanup(state, set, vec);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_insert_present(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            std::vector<T> vec = prepare_vector<T>(prime_index, state);
            Set set(vec.begin(), vec.end());
            state.ResumeTiming();
            for (auto& elem : vec) {
                set.insert(elem);
            }
            cleanup(state, set, vec);
        }
        state.SetComplexityN(state.range(0));
    }


    template<typename Set>
    static void set_insert_hint_smaller_hint(benchmark::State& state) {
        typename Set::value_type greatest{(int) state.range(0) + 1};
        for (auto _ : state) {
            state.PauseTiming();
            Set set;
            typename Set::value_type element{0};
            // To avoid degenerative case when the element to insert will be the greatest element
            // of the set
            set.insert(greatest);
            set.insert(element);
            ++element;
            auto it = set.begin();
            state.ResumeTiming();
            for (int i = 1; i < state.range(0); ++i, ++element) {
                it = set.insert(it, element);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_insert_hint_greater_hint(benchmark::State& state) {
        typename Set::value_type greatest{(int) state.range(0) + 1};
        for (auto _ : state) {
            state.PauseTiming();
            Set set;
            typename Set::value_type element{greatest};
            // To avoid degenerative case when the element to insert will be the smallest element
            // of the set
            set.insert(element);
            --element;
            set.insert(-1);
            auto it = std::prev(set.end());
            state.ResumeTiming();
            for (int i = state.range(0); i >= 0; --i, --element) {
                it = set.insert(it, element);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_erase_absent(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            std::vector<T> vec = prepare_vector<T>(prime_index, state);
            Set set;
            for (auto elem : vec) {
                ++elem;
                set.insert(std::move(elem));
            }
            state.ResumeTiming();
            for (auto& elem : vec) {
                set.erase(elem);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_erase_present(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            std::vector<T> vec = prepare_vector<T>(prime_index, state);
            Set set(vec.begin(), vec.end());
            state.ResumeTiming();
            for (auto& elem : vec) {
                set.erase(elem);
            }
            cleanup(state, set);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_union_of_unbalanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        std::size_t big_size = range_end<T>;
        for (auto _ : state) {
            state.PauseTiming();
            Set smaller = random_insert<Set>(prime_index, big_size * 2, state.range(0));
            Set bigger = random_insert<Set>(prime_index, big_size, big_size);
            state.ResumeTiming();
            if constexpr (std::same_as<Set, std::set<T> >) {
                bigger.merge(std::move(smaller));
                cleanup(state, bigger);
            } else {
                Set result = union_of(std::move(smaller), std::move(bigger));
                cleanup(state, result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_union_of_balanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            Set set1 = random_insert<Set>(prime_index, state.range(0) * 2, state.range(0));
            Set set2 = random_insert<Set>(prime_index, state.range(0) * 2, state.range(0));
            state.ResumeTiming();
            if constexpr (std::same_as<Set, std::set<T> >) {
                set1.merge(std::move(set2));
                cleanup(state, set1);
            } else {
                Set result = union_of(std::move(set1), std::move(set2));
                cleanup(state, result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::set<T> std_set_intersection_of(std::set<T> smaller, std::set<T> bigger) {
        for (auto it = smaller.begin(); it != smaller.end();) {
            if (bigger.contains(*it)) {
                ++it;
            } else {
                it = smaller.erase(it);
            }
        }
        return smaller;
    }

    template<typename Set>
    static void set_intersection_of_unbalanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        std::size_t big_size = range_end<T>;
        for (auto _ : state) {
            state.PauseTiming();
            Set smaller = random_insert<Set>(prime_index, big_size * 2, state.range(0));
            Set bigger = random_insert<Set>(prime_index, big_size, big_size);
            Set result;
            state.ResumeTiming();
            if constexpr (std::same_as<Set, std::set<T> >) {
                result = std_set_intersection_of(std::move(smaller), std::move(bigger));
            } else {
                result = intersection_of(std::move(smaller), std::move(bigger));;
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_intersection_of_balanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            Set set1 = random_insert<Set>(prime_index, state.range(0) * 2, state.range(0));
            Set set2 = random_insert<Set>(prime_index, state.range(0) * 2, state.range(0));
            Set result;
            state.ResumeTiming();
            if constexpr (std::same_as<Set, std::set<T> >) {
                result = std_set_intersection_of(std::move(set1), std::move(set2));
            } else {
                result = intersection_of(std::move(set1), std::move(set2));
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::set<T> std_set_difference_of(std::set<T> original, std::set<T> unwanted) {
        for (auto it = unwanted.begin(); it != unwanted.end(); ++it) {
            auto oit = original.find(*it);
            if (oit != original.end()) {
                original.erase(oit);
            }
        }
        return original;
    }

    template<typename Set>
    static void set_difference_of_unbalanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        std::size_t big_size = range_end<T>;
        for (auto _ : state) {
            state.PauseTiming();
            Set unwanted = random_insert<Set>(prime_index, big_size * 2, state.range(0));
            Set original = random_insert<Set>(prime_index, big_size, big_size);
            Set result;
            state.ResumeTiming();
            if constexpr (std::same_as<Set, std::set<T> >) {
                result = std_set_difference_of(std::move(original), std::move(unwanted));
            } else {
                result = difference_of(std::move(original), std::move(unwanted));
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Set>
    static void set_difference_of_balanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Set::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            Set set1 = random_insert<Set>(prime_index, state.range(0) * 2, state.range(0));
            Set set2 = random_insert<Set>(prime_index, state.range(0) * 2, state.range(0));
            Set result;
            state.ResumeTiming();
            if constexpr (std::same_as<Set, std::set<T> >) {
                result = std_set_difference_of(std::move(set1), std::move(set2));
            } else {
                result = difference_of(std::move(set1), std::move(set2));
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    BENCH_SINGLE(set_begin, small_element, o1);
    BENCH_SINGLE(set_end, small_element, o1);
    BENCH_SINGLE(set_empty, small_element, o1);
    BENCH_SINGLE(set_size, small_element, o1);
    BENCH_SINGLE(set_iterate, small_element, oN);
    BENCH_ALL(set_find, oNLogN);
    BENCH_ALL(set_lower_bound, oNLogN);
    BENCH_ALL(set_upper_bound, oNLogN);
    BENCH_ALL(set_insert_absent, oNLogN);
    BENCH_ALL(set_insert_present, oNLogN);
    BENCH_ALL(set_insert_hint_smaller_hint, oNLogN);
    BENCH_ALL(set_insert_hint_greater_hint, oNLogN);
    BENCH_ALL(set_erase_absent, oNLogN);
    BENCH_ALL(set_erase_present, oNLogN);
    BENCH_SET_OPERATION_UNBALANCED(set_union_of_unbalanced);
    BENCH_SET_OPERATION_BALANCED(set_union_of_balanced);
    BENCH_SET_OPERATION_UNBALANCED(set_intersection_of_unbalanced);
    BENCH_SET_OPERATION_BALANCED(set_intersection_of_balanced);
    BENCH_SET_OPERATION_UNBALANCED(set_difference_of_unbalanced);
    BENCH_SET_OPERATION_BALANCED(set_difference_of_balanced);
}