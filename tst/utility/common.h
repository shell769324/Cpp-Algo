
#pragma once
#include <vector.h>
#include <numeric>
#include <cmath>
#include <concepts>
#include "tst/utility/constructor_stub.h"
#include <unordered_set>

namespace algo {

int random_number(int lo, int hi);

int random_number();

std::vector<constructor_stub> get_random_stub_vector(std::size_t size, int lo = 0, int hi = 10000);

std::vector<std::pair<const constructor_stub, constructor_stub> > get_random_stub_pair_vector(std::size_t size, int lo = 0, int hi = 10000);

std::vector<std::pair<const constructor_stub, constructor_stub> > get_random_stub_pair_vector(std::vector<constructor_stub>& stubs);

class uid_resolver {
    public:
    uid_resolver(bool is_choose_smaller=true) : is_choose_smaller(is_choose_smaller) { }

    bool operator()(const constructor_stub& stub1, const constructor_stub& stub2) const {
        return (stub1.uid < stub2.uid) == is_choose_smaller;
    }

    private:
    bool is_choose_smaller;
};
}
