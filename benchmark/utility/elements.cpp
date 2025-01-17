#include "elements.h"

namespace algo {

small_element::small_element() : key(0) {}

small_element::small_element(long key) : key(key) {}

std::strong_ordering operator<=>(const small_element& s1, const small_element& s2) {
    return s1.key <=> s2.key;
}

small_element& small_element::operator++() {
    ++key;
    return *this;
}

small_element& small_element::operator--() {
    --key;
    return *this;
}

small_element& small_element::operator+=(const small_element& other) {
    key += other.key;
    return *this;
}

medium_element::medium_element() : key(0) {}

medium_element::medium_element(long key) : key(key) {}

std::strong_ordering operator<=>(const medium_element& m1, const medium_element& m2) {
    return m1.key <=> m2.key;
}

medium_element& medium_element::operator+=(const medium_element& other) {
    key += other.key;
    return *this;
}

medium_element& medium_element::operator++() {
    ++key;
    return *this;
}

medium_element& medium_element::operator--() {
    --key;
    return *this;
}

big_element::big_element() : key(0) {}

big_element::big_element(long key) : key(key) {}

std::strong_ordering operator<=>(const big_element& b1, const big_element& b2) {
    return b1.key <=> b2.key;
}

big_element& big_element::operator++() {
    ++key;
    return *this;
}

big_element& big_element::operator--() {
    --key;
    return *this;
}

big_element& big_element::operator+=(const big_element& other) {
    key += other.key;
    return *this;
}

}