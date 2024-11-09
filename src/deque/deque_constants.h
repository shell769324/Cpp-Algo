
#pragma once

#include <cstddef>

namespace algo {
    
constexpr std::size_t MAX_CHUNK_SIZE_BYTES = 1<<9;
constexpr std::size_t MIN_CHUNK_SIZE = 4;
constexpr std::size_t DEFAULT_NUM_CHUNKS = 2;

template<typename T>
constexpr long CHUNK_SIZE = 
        std::max<std::size_t>(MIN_CHUNK_SIZE, MAX_CHUNK_SIZE_BYTES / sizeof(T));
}
