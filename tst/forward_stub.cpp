#include "forward_stub.h"

namespace algo {
    forward_stub::forward_stub() {
        construct_type = constructor_type::default_constructor;
    }

    forward_stub::forward_stub(int n) {
        this -> n = n;
        construct_type = constructor_type::int_constructor;
    }

    forward_stub::forward_stub(float f) {
        this -> f = f;
        construct_type = constructor_type::float_constructor;
    }

    forward_stub::forward_stub(std::string s) {
        this -> s = s;
        construct_type = constructor_type::string_constructor;
    }
}
