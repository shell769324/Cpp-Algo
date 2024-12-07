#pragma once

namespace algo {
class construction_destruction_tracker {
private:
    std::size_t constructor_mark;
    std::size_t constructed_mark;
    std::size_t destructor_mark;
    std::size_t destroyed_mark;
    std::size_t pair_constructed_mark;
    std::size_t pair_destroyed_mark;

public:
    void reset();
    void check();
    void mark();
    void check_marked();
};
}