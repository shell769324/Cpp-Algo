
#pragma once

#include <cstddef>
#include <algorithm>

namespace algo {
    
constexpr static std::size_t MAX_CHUNK_SIZE_BYTES = 1<<9;
constexpr static std::size_t MIN_CHUNK_SIZE = 4;
constexpr static std::size_t CHUNK_PADDING = 2;

template<typename T>
constexpr long CHUNK_SIZE = 
        std::max<long>(MIN_CHUNK_SIZE, MAX_CHUNK_SIZE_BYTES / sizeof(T));
}
