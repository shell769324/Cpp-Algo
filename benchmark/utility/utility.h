#pragma once
#include <benchmark/benchmark.h>

template <typename T>
static void wipe(T&& value) {
    T tmp = std::move(value);
}

template <typename T, typename... Args>
static void wipe(T&& value, Args&&... args) {
    T tmp = std::move(value);
    wipe(std::move(args)...);
}

template <typename... Args>
void cleanup(benchmark::State& state, Args&&... args) {
    state.PauseTiming();
    wipe(std::move(args)...);
    state.ResumeTiming();
}