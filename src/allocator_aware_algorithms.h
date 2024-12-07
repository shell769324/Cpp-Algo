#pragma once
#include <concepts>
#include <iterator>
#include <memory>

namespace algo {

template<typename Iterator, typename Allocator>
requires std::weakly_incrementable<Iterator> && std::indirectly_readable<Iterator> && std::equality_comparable<Iterator>
void destroy(Iterator first, Iterator last, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    for (; first != last; ++first) {
        alloc_traits::destroy(allocator, std::addressof(*first));
    }
}

template<std::forward_iterator ForwardIt, typename Allocator>
void uninitialized_default_construct(ForwardIt first, ForwardIt last, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    ForwardIt current = first;
    try {
        for (; current != last; ++current) {
            alloc_traits::construct(allocator, std::addressof(*current));
        }
    } catch (...) {
        destroy(first, current, allocator);
        throw;
    }
}

template<std::forward_iterator ForwardIt, typename T, typename Allocator>
void uninitialized_fill(ForwardIt first, ForwardIt last, const T& value, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    ForwardIt current = first;
    try {
        for (; current != last; ++current) {
            alloc_traits::construct(allocator, std::addressof(*current), value);
        }
    } catch (...) {
        destroy(first, current, allocator);
        throw;
    }
}

template<std::input_iterator InputIt, std::forward_iterator NoThrowForwardIt, typename Allocator>
NoThrowForwardIt uninitialized_copy(InputIt first, InputIt last, NoThrowForwardIt d_first, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    NoThrowForwardIt current = d_first;
    try {
        for (; first != last; ++first, ++current) {
            alloc_traits::construct(allocator, std::addressof(*current), *first);
        }
        return current;
    } catch (...) {
        destroy(d_first, current, allocator);
        throw;
    }
}

template<std::bidirectional_iterator NoThrowBidirIt, typename Allocator>
NoThrowBidirIt uninitialized_copy_backwards(NoThrowBidirIt first, NoThrowBidirIt last, NoThrowBidirIt d_last, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    NoThrowBidirIt current = d_last;
    try {
        while (first != last) {
            alloc_traits::construct(allocator, std::addressof(*(--current)), *(--last));
        }
        return current;
    } catch (...) {
        // Current is the pos where we failed to construct. It is guaranteed to be one smaller than d_last
        --d_last;
        while (current != d_last) {
            alloc_traits::destroy(allocator, std::addressof(*d_last));
        }
        throw;
    }
}

template<std::input_iterator InputIt, std::forward_iterator NoThrowForwardIt, typename Allocator>
NoThrowForwardIt uninitialized_move(InputIt first, InputIt last, NoThrowForwardIt d_first, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    NoThrowForwardIt current = d_first;
    try {
        for (; first != last; ++first, ++current) {
            alloc_traits::construct(allocator, std::addressof(*current), std::move(*first));
        }
        return current;
    } catch (...) {
        destroy(d_first, current, allocator);
        throw;
    }
}

template<std::bidirectional_iterator NoThrowBidirIt, typename Allocator>
NoThrowBidirIt uninitialized_move_backwards(NoThrowBidirIt first, NoThrowBidirIt last, NoThrowBidirIt d_last, Allocator& allocator) {
    using alloc_traits = std::allocator_traits<Allocator>;
    NoThrowBidirIt current = d_last;
    try {
        while (first != last) {
            alloc_traits::construct(allocator, std::addressof(*(--current)), std::move(*(--last)));
        }
        return current;
    } catch (...) {
        // Current is the pos where we failed to construct. It is guaranteed to be one smaller than d_last
        --d_last;
        while (current != d_last) {
            alloc_traits::destroy(allocator, std::addressof(*d_last));
        }
        throw;
    }
}

};