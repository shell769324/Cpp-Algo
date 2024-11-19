#include <memory>

template <typename T, std::size_t ChunkSize>
class chunk_allocator {
    using value_type == T;
    using size_type = std::size_t
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    /*
     * A vector that holds a list of pages
     *
     * A sorted map from sizes of available range to the vector of begins
     */
    chunk_allocator() {
        
    }

    constexpr value_type* allocate(std::size_t n) {
        return nullptr;
    }

    constexpr void deallocate(T* p, std::size_t n) {

    }
};
