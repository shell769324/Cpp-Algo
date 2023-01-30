#include "common.h"

namespace algo {

int random_number(int lo, int hi) {
    return rand() % (hi - lo) + lo;
}

int random_number() {
    return random_number(0, 100000);
}


std::vector<constructor_stub> get_random_stub_vector(std::size_t size, int lo, int hi) {
    std::unordered_set<int> ids;
    while (ids.size() < size) {
        ids.insert(random_number(lo, hi));
    }
    return std::vector<constructor_stub> (ids.cbegin(), ids.cend());
}

std::vector<std::pair<const constructor_stub, constructor_stub> > get_random_stub_pair_vector(std::size_t size, int lo, int hi) {
    std::unordered_set<int> ids;
    while (ids.size() < size) {
        ids.insert(random_number(lo, hi));
    }
    std::vector<std::pair<const constructor_stub, constructor_stub> > res;
    for (int num : ids) {
        res.emplace_back(constructor_stub(num), constructor_stub(-num));
    }
    return res;
}

std::vector<std::pair<const constructor_stub, constructor_stub> > get_random_stub_pair_vector(std::vector<constructor_stub>& stubs) {
    std::vector<std::pair<const constructor_stub, constructor_stub> > res;
    for (constructor_stub& stub : stubs) {
        res.emplace_back(stub, constructor_stub(-stub.id));
    }
    return res;
}
}
