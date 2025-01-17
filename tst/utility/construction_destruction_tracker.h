#pragma once

namespace algo {
class construction_destruction_tracker {
public:
    std::size_t constructor_mark;
    std::size_t constructed_mark;
    std::size_t destructor_mark;
    std::size_t destroyed_mark;
    std::size_t pair_constructed_mark;
    std::size_t pair_destroyed_mark;

    void reset();
    void check();
    void mark();
    void check_marked();
};
}