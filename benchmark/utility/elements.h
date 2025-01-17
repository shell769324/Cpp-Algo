#pragma once
#include <compare>

namespace algo {
struct small_element {
    long key;
    small_element();
    small_element(long key);
    small_element& operator++();
    small_element& operator--();
    small_element& operator+=(const small_element& other);
    friend std::strong_ordering operator<=>(const small_element& s1, const small_element& s2);
};

struct medium_element {
    long key;
    char myArray[50];
    medium_element();
    medium_element(long key);
    medium_element& operator++();
    medium_element& operator--();
    medium_element& operator+=(const medium_element& other);
    friend std::strong_ordering operator<=>(const medium_element& s1, const medium_element& s2);
};

struct big_element {
    long key;
    char myArray[500];
    big_element();
    big_element(long key);
    big_element& operator++();
    big_element& operator--();
    big_element& operator+=(const big_element& other);
    friend std::strong_ordering operator<=>(const big_element& s1, const big_element& s2);
};
}