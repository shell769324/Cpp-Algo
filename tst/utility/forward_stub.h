#pragma once
#include <string>

namespace algo {
class forward_stub {
public:
    enum class constructor_type {default_constructor, int_constructor, string_constructor, float_constructor};

    constructor_type construct_type;
    int n;
    float f;
    std::string s;

    forward_stub();

    forward_stub(int n);

    forward_stub(float f);

    forward_stub(std::string s);
};
}
