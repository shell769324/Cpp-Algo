#include <benchmark/benchmark.h>
#include "src/tree/avl_tree_map.h"
#include "src/tree/red_black_tree_map.h"
#include <map>
#include "utility/elements.h"
#include "utility/utility.h"
#include "utility/tree_utility.h"
#include <vector>
#include <cmath>

namespace {
    using namespace algo;
    static std::vector<std::string> produce_keys();
    
    const static std::vector<std::string> keys = produce_keys();

    static std::vector<std::string> produce_keys() {
        std::vector<std::string> result;
        int total = 2 * range_end<int>;
        result.reserve(total);
        std::vector<char> char_map;
        for (char c = '!'; c <= '~'; ++c) {
            char_map.push_back(c);
        }
        for (int i = 0; i < total; ++i) {
            std::string curr;
            int remain = i;
            while (remain > 0) {
                curr.push_back(char_map[remain % char_map.size()]);
                remain /= char_map.size();
            }
            result.push_back(curr);
        }
        return result;
    }

    #define BENCH_SINGLE(name, type, bigo) \
        BENCHMARK(name<avl_tree_map<std::string, type>>)->Name("avl_tree_map/" #name "/" #type)->RangeMultiplier(2)->Range(range_begin<type>, range_end<type>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<red_black_tree_map<std::string, type>>)->Name("red_black_tree_map/" #name "/" #type)->RangeMultiplier(2)->Range(range_begin<type>, range_end<type>)->Complexity(benchmark::bigo); \
        BENCHMARK(name<std::map<std::string, type>>)->Name("std/" #name "/" #type)->RangeMultiplier(2)->Range(range_begin<type>, range_end<type>)->Complexity(benchmark::bigo)

    #define BENCH_ALL(name, bigo) \
        BENCH_SINGLE(name, int, bigo); \
        BENCH_SINGLE(name, small_element, bigo); \
        BENCH_SINGLE(name, medium_element, bigo); \
        BENCH_SINGLE(name, big_element, bigo)

    // Use a fix number of iterations since setup and teardown takes much longer than the measured portion of 
    // the benchmark. Google benchmark sets the number of iterations based on the measured portion, not the entirety
    // of the code in the loop.
    #define BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, type) \
        BENCHMARK(name<avl_tree_map<std::string, type>>)->Name("avl_tree_map/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(set_op_complexity<type>); \
        BENCHMARK(name<red_black_tree_map<std::string, type>>)->Name("red_black_tree_map/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(set_op_complexity<type>); \
        BENCHMARK(name<std::map<std::string, type>>)->Name("std/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(std_set_op_complexity<type>)

    #define BENCH_SET_OPERATION_UNBALANCED(name) \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, int); \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, small_element); \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, medium_element); \
        BENCH_SINGLE_SET_OPERATION_UNBALANCED(name, big_element)

    #define BENCH_SINGLE_SET_OPERATION_BALANCED(name, type) \
        BENCHMARK(name<avl_tree_map<std::string, type>>)->Name("avl_tree_map/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(benchmark::oN); \
        BENCHMARK(name<red_black_tree_map<std::string, type>>)->Name("red_black_tree_map/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(benchmark::oN); \
        BENCHMARK(name<std::map<std::string, type>>)->Name("std/" #name "/" #type)->RangeMultiplier(2)-> \
            Range(range_begin<type>, range_end<type>)->Iterations(set_operation_iter)->Complexity(benchmark::oNLogN)

    #define BENCH_SET_OPERATION_BALANCED(name) \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, int); \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, small_element); \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, medium_element); \
        BENCH_SINGLE_SET_OPERATION_BALANCED(name, big_element)

    template<typename Map>
    static void map_begin(benchmark::State& state) {
        Map map;
        for (int i = 0; i < state.range(0); ++i) {
            typename Map::mapped_type element{i};
            map.emplace(keys[i], element);
        }
        for (auto _ : state) {
            auto it = map.begin();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_end(benchmark::State& state) {
        Map map;
        for (int i = 0; i < state.range(0); ++i) {
            typename Map::mapped_type element{i};
            map.emplace(keys[i], element);
        }
        for (auto _ : state) {
            auto it = map.end();
            benchmark::DoNotOptimize(it);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_empty(benchmark::State& state) {
        Map map;
        for (int i = 0; i < state.range(0); ++i) {
            typename Map::mapped_type element{i};
            map.emplace(keys[i], element);
        }
        for (auto _ : state) {
            bool is_empty = map.empty();
            benchmark::DoNotOptimize(is_empty);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_size(benchmark::State& state) {
        Map map;
        for (int i = 0; i < state.range(0); ++i) {
            typename Map::mapped_type element{i};
            map.emplace(keys[i], element);
        }
        for (auto _ : state) {
            std::size_t size = map.size();
            benchmark::DoNotOptimize(size);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    Map random_insert(int& prime_index, int limit, int count) {
        Map map;
        // Randomize insertion order
        // Also half elements in the range is missing
        for (int i = 0, val = 0; i < count; ++i, val = (val + primes[prime_index]) % limit) {
            typename Map::mapped_type element{val};
            map.emplace(keys[val], element);
        }
        prime_index = (prime_index + 1) % primes.size();
        return map;
    }

    template<typename Map>
    Map prepare_map(int& prime_index, benchmark::State& state) {
        state.PauseTiming();
        int half_range = state.range(0) / 2;
        Map map = random_insert<Map>(prime_index, state.range(0), half_range);
        state.ResumeTiming();
        return map;
    }

    template<typename Map>
    static void map_iterate(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Map map = prepare_map<Map>(prime_index, state);
            typename Map::mapped_type element{0};
            for (auto& elem : map) {
                element += elem.second;
            }
            benchmark::DoNotOptimize(element);
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_find(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Map map = prepare_map<Map>(prime_index, state);
            for (int i = 0; i < state.range(0); ++i) {
                auto it = map.find(keys[i]);
                benchmark::DoNotOptimize(it);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_access(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Map map = prepare_map<Map>(prime_index, state);
            for (int i = 0; i < state.range(0); ++i) {
                auto& value = map[keys[i]];
                benchmark::DoNotOptimize(value);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_lower_bound(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Map map = prepare_map<Map>(prime_index, state);
            for (int i = 0; i < state.range(0); ++i) {
                auto it = map.lower_bound(keys[i]);
                benchmark::DoNotOptimize(it);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_upper_bound(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            Map map = prepare_map<Map>(prime_index, state);
            for (int i = 0; i < state.range(0); ++i) {
                auto it = map.upper_bound(keys[i]);
                benchmark::DoNotOptimize(it);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::vector<std::pair<const std::string, T> > 
    prepare_vector(int& prime_index, benchmark::State& state) {
        std::vector<std::pair<const std::string, T> > vec;
        // Randomize insertion order
        for (int i = 0, val = 0; i < state.range(0); ++i, val = (val + primes[prime_index]) % state.range(0)) {
            vec.emplace_back(keys[val], T{val});
        }
        prime_index = (prime_index + 1) % primes.size();
        return vec;
    }

    template<typename Map>
    static void map_insert_absent(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            std::vector<T> vec = prepare_vector<typename Map::mapped_type>(prime_index, state);
            Map map;
            state.ResumeTiming();
            for (auto& elem : vec) {
                map.insert(elem);
            }
            cleanup(state, map, vec);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_insert_present(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::value_type;
        for (auto _ : state) {
            state.PauseTiming();
            std::vector<T> vec = prepare_vector<typename Map::mapped_type>(prime_index, state);
            Map map(vec.begin(), vec.end());
            state.ResumeTiming();
            for (auto& elem : vec) {
                map.insert(elem);
            }
            cleanup(state, map, vec);
        }
        state.SetComplexityN(state.range(0));
    }


    template<typename Map>
    static void map_emplace_hint_smaller_hint(benchmark::State& state) {
        std::string greatest_key = keys[state.range(0) + 1];
        using T = typename Map::mapped_type;
        T greatest{(int) state.range(0) + 1};
        for (auto _ : state) {
            state.PauseTiming();
            Map map;
            T element{0};
            // To avoid degenerative case when the element to insert will be the greatest element
            // of the map
            map.emplace(greatest_key, greatest);
            map.emplace(keys[0], element);
            ++element;
            auto it = map.begin();
            state.ResumeTiming();
            for (int i = 1; i < state.range(0); ++i, ++element) {
                it = map.emplace_hint(it, keys[i], element);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_emplace_hint_greater_hint(benchmark::State& state) {
        std::string greatest_key = keys[state.range(0) + 1];
        using T = typename Map::mapped_type;
        for (auto _ : state) {
            state.PauseTiming();
            Map map;
            T element{(int) state.range(0) + 1};
            // To avoid degenerative case when the element to insert will be the smallest element
            // of the map
            map.emplace(greatest_key, element);
            --element;
            map.emplace(keys[0], T{0});
            auto it = std::prev(map.end());
            state.ResumeTiming();
            for (int i = state.range(0); i >= 1; --i, --element) {
                it = map.emplace_hint(it, keys[i], element);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::vector<std::pair<int, T> > 
    prepare_erase_vector(int& prime_index, benchmark::State& state) {
        std::vector<std::pair<int, T> > vec;
        // Randomize insertion order
        for (int i = 0, val = 0; i < state.range(0); ++i, val = (val + primes[prime_index]) % state.range(0)) {
            int doubled = val * 2;
            vec.emplace_back(doubled, T{doubled});
        }
        prime_index = (prime_index + 1) % primes.size();
        return vec;
    }

    template<typename Map>
    static void map_erase_absent(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            state.PauseTiming();
            auto vec = prepare_erase_vector<typename Map::mapped_type>(prime_index, state);
            Map map;
            for (auto elem : vec) {
                map.emplace(keys[elem.first + 1], elem.second);
            }
            state.ResumeTiming();
            for (auto& elem : vec) {
                map.erase(keys[elem.first]);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_erase_present(benchmark::State& state) {
        int prime_index = 0;
        for (auto _ : state) {
            state.PauseTiming();
            auto vec = prepare_erase_vector<typename Map::mapped_type>(prime_index, state);
            Map map;
            for (auto elem : vec) {
                map.emplace(keys[elem.first], elem.second);
            }
            state.ResumeTiming();
            for (auto& elem : vec) {
                map.erase(keys[elem.first]);
            }
            cleanup(state, map);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_union_of_unbalanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::mapped_type;
        std::size_t big_size = range_end<T>;
        for (auto _ : state) {
            state.PauseTiming();
            Map smaller = random_insert<Map>(prime_index, big_size * 2, state.range(0));
            Map bigger = random_insert<Map>(prime_index, big_size, big_size);
            state.ResumeTiming();
            if constexpr (std::same_as<Map, std::map<std::string, T> >) {
                bigger.merge(std::move(smaller));
                cleanup(state, bigger);
            } else {
                Map result = union_of(std::move(smaller), std::move(bigger));
                cleanup(state, result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_union_of_balanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::mapped_type;
        for (auto _ : state) {
            state.PauseTiming();
            Map set1 = random_insert<Map>(prime_index, state.range(0) * 2, state.range(0));
            Map set2 = random_insert<Map>(prime_index, state.range(0) * 2, state.range(0));
            state.ResumeTiming();
            if constexpr (std::same_as<Map, std::map<std::string, T> >) {
                set1.merge(std::move(set2));
                cleanup(state, set1);
            } else {
                Map result = union_of(std::move(set1), std::move(set2));
                cleanup(state, result);
            }
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::map<std::string, T> std_set_intersection_of(std::map<std::string, T> smaller, 
                                                     std::map<std::string, T> bigger) {
        for (auto it = smaller.begin(); it != smaller.end();) {
            if (bigger.contains(it -> first)) {
                ++it;
            } else {
                it = smaller.erase(it);
            }
        }
        return smaller;
    }

    template<typename Map>
    static void map_intersection_of_unbalanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::mapped_type;
        std::size_t big_size = range_end<T>;
        for (auto _ : state) {
            state.PauseTiming();
            Map smaller = random_insert<Map>(prime_index, big_size * 2, state.range(0));
            Map bigger = random_insert<Map>(prime_index, big_size, big_size);
            Map result;
            state.ResumeTiming();
            if constexpr (std::same_as<Map, std::map<std::string, T> >) {
                result = std_set_intersection_of(std::move(smaller), std::move(bigger));
            } else {
                result = intersection_of(std::move(smaller), std::move(bigger));;
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_intersection_of_balanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::mapped_type;
        for (auto _ : state) {
            state.PauseTiming();
            Map set1 = random_insert<Map>(prime_index, state.range(0) * 2, state.range(0));
            Map set2 = random_insert<Map>(prime_index, state.range(0) * 2, state.range(0));
            Map result;
            state.ResumeTiming();
            if constexpr (std::same_as<Map, std::map<std::string, T> >) {
                result = std_set_intersection_of(std::move(set1), std::move(set2));
            } else {
                result = intersection_of(std::move(set1), std::move(set2));
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename T>
    std::map<std::string, T> std_set_difference_of(std::map<std::string, T> original, 
                                                   std::map<std::string, T> unwanted) {
        for (auto it = unwanted.begin(); it != unwanted.end(); ++it) {
            auto oit = original.find(it -> first);
            if (oit != original.end()) {
                original.erase(oit);
            }
        }
        return original;
    }

    template<typename Map>
    static void map_difference_of_unbalanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::mapped_type;
        std::size_t big_size = range_end<T>;
        for (auto _ : state) {
            state.PauseTiming();
            Map unwanted = random_insert<Map>(prime_index, big_size * 2, state.range(0));
            Map original = random_insert<Map>(prime_index, big_size, big_size);
            Map result;
            state.ResumeTiming();
            if constexpr (std::same_as<Map, std::map<std::string, T> >) {
                result = std_set_difference_of(std::move(original), std::move(unwanted));
            } else {
                result = difference_of(std::move(original), std::move(unwanted));
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    template<typename Map>
    static void map_difference_of_balanced(benchmark::State& state) {
        int prime_index = 0;
        using T = typename Map::mapped_type;
        for (auto _ : state) {
            state.PauseTiming();
            Map set1 = random_insert<Map>(prime_index, state.range(0) * 2, state.range(0));
            Map set2 = random_insert<Map>(prime_index, state.range(0) * 2, state.range(0));
            Map result;
            state.ResumeTiming();
            if constexpr (std::same_as<Map, std::map<std::string, T> >) {
                result = std_set_difference_of(std::move(set1), std::move(set2));
            } else {
                result = difference_of(std::move(set1), std::move(set2));
            }
            cleanup(state, result);
        }
        state.SetComplexityN(state.range(0));
    }

    BENCH_SINGLE(map_begin, small_element, o1);
    BENCH_SINGLE(map_end, small_element, o1);
    BENCH_SINGLE(map_empty, small_element, o1);
    BENCH_SINGLE(map_size, small_element, o1);
    BENCH_SINGLE(map_iterate, small_element, oN);
    BENCH_ALL(map_find, oNLogN);
    BENCH_ALL(map_access, oNLogN);
    BENCH_ALL(map_lower_bound, oNLogN);
    BENCH_ALL(map_upper_bound, oNLogN);
    BENCH_ALL(map_insert_absent, oNLogN);
    BENCH_ALL(map_insert_present, oNLogN);
    BENCH_ALL(map_emplace_hint_smaller_hint, oNLogN);
    BENCH_ALL(map_emplace_hint_greater_hint, oNLogN);
    BENCH_ALL(map_erase_absent, oNLogN);
    BENCH_ALL(map_erase_present, oNLogN);
    BENCH_SET_OPERATION_UNBALANCED(map_union_of_unbalanced);
    BENCH_SET_OPERATION_BALANCED(map_union_of_balanced);
    BENCH_SET_OPERATION_UNBALANCED(map_intersection_of_unbalanced);
    BENCH_SET_OPERATION_BALANCED(map_intersection_of_balanced);
    BENCH_SET_OPERATION_UNBALANCED(map_difference_of_unbalanced);
    BENCH_SET_OPERATION_BALANCED(map_difference_of_balanced);
}