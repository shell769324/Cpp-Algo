#pragma once
#include "src/allocator_aware_algorithms.h"
#include <optional>

namespace algo {

template <typename T, typename Operator, typename Allocator = std::allocator<T> >
concept segment_tree_definable =
    std::regular_invocable<Operator, T, T> && std::same_as<T, std::decay_t<std::invoke_result_t<Operator, T, T> > > 
    && std::same_as<T, typename Allocator::value_type>;

template <typename T, typename Operator, typename RepeatOperator, typename Allocator = std::allocator<T> >
concept range_segment_tree_definable =
    std::regular_invocable<Operator, T, T> && std::regular_invocable<RepeatOperator, std::size_t, T> 
    && std::same_as<T, std::decay_t<std::invoke_result_t<Operator, T, T> > > 
    && std::same_as<T, std::decay_t<std::invoke_result_t<RepeatOperator, std::size_t, T> > >
    && std::same_as<T, typename Allocator::value_type>;

template <typename T, typename Operator, typename Allocator = std::allocator<T> >
requires segment_tree_definable<T, Operator, Allocator>
class segment_tree;

template <typename T, typename Operator, typename Allocator = std::allocator<T> >
class segment_tree_initializer {
private:
    std::size_t& length;
    T*& data;
    Operator& op;
    Allocator& allocator;

    using alloc_traits = std::allocator_traits<Allocator>;
    using cleaner_type = std::unique_ptr<T, deleter<T, Allocator> >;


    /**
     * @brief Build a segment tree recursively
     * 
     * @tparam B the boolean value type to indicate if we are building with an input iterator. Used for tag dispatch purpose
     * @tparam Args the types of the value parameter pack to construct leaf element in-place
     * @param left the begin of the interval to apply op
     * @param right the end of the interval to apply op
     * @param curr the root of the current subtree
     * @param args the value parameter pack to construct leaf element in-place
     * @return T the result of applying op over the given interval
     */
    template<typename IsIterator, typename... Args>
    T* build(std::size_t left, std::size_t right, T* curr, Args&&... args) {
        if (right - left == 1) {
            if constexpr (std::same_as<std::true_type, IsIterator>) {
                auto& it = [](auto& first, auto&...) -> auto& { return first; }(args...);
                alloc_traits::construct(allocator, curr, *it);
                ++it;
            } else {
                alloc_traits::construct(allocator, curr, std::forward<Args>(args)...);
            }
            return curr;
        }
        std::size_t mid = (right - left) / 2 + left;
        T* destroy_begin = curr + 1;
        T* destroy_end = destroy_begin;
        try {
            // Destroying elements should be owned by the recursive call
            T* left_root = build<IsIterator>(left, mid, curr + 1, std::forward<Args>(args)...);
            // If a need rises to destroy them after we exit a child recursive call, it is the parent's responsibility
            // to clean up the child range
            destroy_end = curr + 2 * (mid - left);
            // 2 * (mid - left) because that the subtree rooted at curr + 1 will use a subarray of size 2 * (mid - left) - 1
            T* right_root = build<IsIterator>(mid, right, destroy_end, std::forward<Args>(args)...);
            destroy_end = curr + 2 * (right - left) - 1;
            alloc_traits::construct(allocator, curr, op(*left_root, *right_root));
        } catch (...) {
            destroy(destroy_begin, destroy_end, allocator);
            throw;
        }
        return curr;
    }

    std::size_t get_data_length() const noexcept {
        return length * 2 - 1;
    }

    /**
     * @brief Allocate the raw buffer with size 2 * length - 1
     * 
     * @return cleaner_type a unique_ptr that will deallocate the buffer
     */
    cleaner_type allocate_data() {
        std::size_t data_length = get_data_length();
        data = alloc_traits::allocate(allocator, data_length);
        return cleaner_type(data, deleter<T, Allocator>(data_length, allocator));
    }

    template<typename... Args>
    void repeat_initialize_helper(Args&&... args) {
        cleaner_type cleaner = allocate_data();
        build<std::false_type>(0, length, data, std::forward<Args>(args)...);
        cleaner.release();
    }

    segment_tree_initializer(std::size_t& length, T*& data, Operator& op, Allocator& allocator)
        : length(length), data(data), op(op), allocator(allocator) { }


    friend class segment_tree<T, Operator, Allocator>;

    template <typename U, typename AnyOperator, typename RepeatOperator, typename AnyAllocator>
    requires range_segment_tree_definable<U, AnyOperator, RepeatOperator, AnyAllocator>
    friend class range_segment_tree;
};

template<typename>
struct segment_tree_decidier {
    using type = void;
};


template <typename... Args>
struct segment_tree_decidier<segment_tree<Args...>> {
    using type = std::true_type;
};

template <typename Tree>
struct segment_tree_traits {
    using alloc_traits = std::allocator_traits<typename Tree::allocator_type>;
    using subtree_type = std::tuple<typename Tree::pointer, std::size_t, std::size_t>;
    using subtree_allocator_type = typename alloc_traits::template rebind_alloc<subtree_type>;
    using subtree_unique_ptr = std::unique_ptr<subtree_type[], deleter<subtree_type, subtree_allocator_type> >;
};

/**
 * @brief Compute the result of applying op over the range of [first, last)
 * 
 * Runs in log(length) time
 * 
 * @pre [left, right) contains [first, last)
 * @param first the begin of the query range
 * @param last the end of the query range
 * @param curr the root of the current subtree
 * @param left the begin of the range that represents the current subtree
 * @param right the end of the range that represents the current subtree
 * @return T the query result
 */
template<typename Tree, typename T>
T query_helper(const Tree& tree, std::size_t first, std::size_t last, const T* curr, std::size_t left, std::size_t right) {
    if (first == left && last == right) {
        return *curr;
    }
    if constexpr (std::is_void_v<typename segment_tree_decidier<Tree>::type>) {
        (const_cast<Tree&>(tree)).push((const_cast<T*>(curr)), left, right);
    }
    std::size_t mid = ((right - left) >> 1) + left;
    // The left half contains the query range wholly
    if (last <= mid) {
        return query_helper(tree, first, last, curr + 1, left, mid);
    }
    // The right half contains the query range wholly
    if (first >= mid) {
        // 2 * (mid - left) because that the subtree rooted at curr + 1 will use a subarray of size 2 * (mid - left) - 1
        return query_helper(tree, first, last, curr + 2 * (mid - left), mid, right);
    }
    return tree.op(query_helper(tree, first, mid, curr + 1, left, mid), 
                   query_helper(tree, mid, last, curr + 2 * (mid - left), mid, right));
}

template <typename T>
std::size_t count_subtrees(T* curr, std::size_t left, std::size_t right, std::size_t first, std::size_t last) noexcept {
    T* left_root = curr + 1;
    std::size_t mid = ((right - left) >> 1) + left;
    T* right_root = curr + 2 * (mid - left);
    if (left == first && right == last) {
        return 1;
    }
    if (last <= mid) {
        return count_subtrees(left_root, left, mid, first, last);
    }
    if (mid <= first) {
        return count_subtrees(right_root, mid, right, first, last);
    }
    return count_subtrees(left_root, left, mid, first, mid)
        + count_subtrees(right_root, mid, right, mid, last);
}

/**
 * @brief Collect an ordered list of disjointed intervals that represent subtrees such that their union equals to
 *        the given query range
 * 
 * @pre [left, right) contains [first, last)
 * @param fill_pos the position to construct the next subtree
 * @param first the begin of the query range
 * @param last the end of the query range
 * @param curr the root of the current subtree
 * @param left the begin of the range that represents the current subtree
 * @param right the end of the range that represents the current subtree
 */
template <typename Tree, typename T>
void collect_subtrees_helper(Tree& tree, std::tuple<T*, std::size_t, std::size_t>*& fill_pos, T* curr, std::size_t left, std::size_t right, 
                             std::size_t first, std::size_t last) noexcept {
    T* left_root = curr + 1;
    std::size_t mid = ((right - left) >> 1) + left;
    T* right_root = curr + 2 * (mid - left);
    if (left == first && right == last) {
        std::construct_at(fill_pos, curr, left, right);
        ++fill_pos;
        return;
    }
    if constexpr (std::is_void_v<typename segment_tree_decidier<Tree>::type>) {
        (const_cast<Tree&>(tree)).push(curr, left, right);
    }
    if (last <= mid) {
        collect_subtrees_helper(tree, fill_pos, left_root, left, mid, first, last);
        return;
    }
    if (mid <= first) {
        collect_subtrees_helper(tree, fill_pos, right_root, mid, right, first, last);
        return;
    }
    collect_subtrees_helper(tree, fill_pos, left_root, left, mid, first, mid);
    collect_subtrees_helper(tree, fill_pos, right_root, mid, right, mid, last);
}

/**
 * @brief Collect an ordered list of disjointed intervals that represent subtrees such that their union equals to 
 *        the given query range
 * 
 * Runs in log(length) time where length is the number of elements held in the tree
 * 
 * @param tree the tree to collect its subtrees from
 * @param first the begin of the query range
 * @param last the end of the query range
 * @return an ordered list of disjointed intervals that represent subtrees such that their union equals to 
 *         the given query range
 */
template <typename Tree>
segment_tree_traits<Tree>::subtree_unique_ptr collect_subtrees(Tree& tree, std::size_t first, std::size_t last) {
    // Two passes can allow use to allocate a fixed memory. The performance loss
    // is justified
    std::size_t subtree_count = count_subtrees(tree.data, 0, tree.length, first, last);
    using traits = segment_tree_traits<Tree>;
    using subtree_type = typename traits::subtree_type;
    subtree_type* subtrees = traits::alloc_traits::template rebind_traits<subtree_type>::allocate(tree.subtree_allocator, subtree_count);
    // subtrees will be incremented, so we need to save a pointer to the start here
    subtree_type* result = subtrees;
    collect_subtrees_helper(tree, subtrees, tree.data, 0, tree.length, first, last);
    return typename traits::subtree_unique_ptr(result, deleter<subtree_type, typename traits::subtree_allocator_type>(subtree_count, tree.subtree_allocator));
}


/**
 * @brief Get the index of the exclusive end of the smallest prefix that satisfies the given condition
 *        in a subtree
 * @tparam F the type of the predicate for prefix result
 * @param op the associated binary operator
 * @param curr the root of the current subtree
 * @param left the begin of the range that represents the current subtree
 * @param right the end of the range that represents the current subtree
 * @param acc an optional value of the accumulated value
 * @param decider the predicate for prefix result
 * @return the exclusive end of the smallest prefix that satisfies the given condition in a subtree
 */
template<typename F, typename T, typename Tree>
std::size_t prefix_search_helper(Tree& tree, T* curr, std::size_t left, std::size_t right, std::optional<T> acc, F& decider) {
    if (right - left == 1) {
        return left + 1;
    }
    if constexpr (std::is_void_v<typename segment_tree_decidier<Tree>::type>) {
        (const_cast<Tree&>(tree)).push(curr, left, right);
    }
    T* left_root = curr + 1;
    std::size_t mid = (right - left) / 2 + left;
    T* right_root = curr + 2 * (mid - left);
    if ((acc && decider(tree.op(*acc, *left_root))) || decider(*left_root)) {
        return prefix_search_helper(tree, left_root, left, mid, try_move(acc), decider);
    }
    std::optional<T> new_acc = std::make_optional(acc ? tree.op(*acc, *left_root) : *left_root);
    return prefix_search_helper(tree, right_root, mid, right, try_move(new_acc), decider);
}

/**
 * @brief Get an optional index of the exclusive end of the smallest prefix that satisfies the given condition in a range
 * 
 * Runs in log(n) time where n is the size of the segment tree
 * 
 * @pre decider must meet the requirement that decider(op(data[:i])) implies decider(op(data[:j])) where i < j.
 *      Most common operations such as addition, min and max all satisfy this requirement.
 * @tparam F the type of the predicate for prefix result.
 * @param tree the tree to search the prefix of
 * @param decider the predicate for prefix result
 * @param first the inclusive begin of the range
 * @param last the exclusive end of the range
 * @return the index if such prefix of the given range exists, or else an empty optional
 */
template<typename Tree, typename F>
std::optional<std::size_t> prefix_search(Tree& tree, F& decider, std::size_t first, std::size_t last) {
    if (first == last) {
        std::optional<std::size_t>();
    }
    typename segment_tree_traits<Tree>::subtree_unique_ptr subtrees = collect_subtrees(tree, first, last);
    std::size_t subtree_count = subtrees.get_deleter().size();
    using T = typename Tree::value_type;
    std::optional<T> acc;
    for (std::size_t i = 0; i < subtree_count; ++i) {
        std::optional<T> new_acc(acc ? tree.op(acc.value(), *std::get<0>(subtrees[i])) : *std::get<0>(subtrees[i]));
        if (decider(new_acc.value())) {
            return std::make_optional(prefix_search_helper(tree, std::get<0>(subtrees[i]), std::get<1>(subtrees[i]), 
                std::get<2>(subtrees[i]), try_move(acc), decider));
        }
        acc = try_move(new_acc);
    }
    return std::optional<std::size_t>();
}

/**
 * @brief Get the index of the inclusive start of the smallest suffix that satisfies the given condition
 *        in a subtree
 * @tparam F the type of the predicate for suffix result
 * @param op the associated binary operator
 * @param curr the root of the current subtree
 * @param left the begin of the range that represents the current subtree
 * @param right the end of the range that represents the current subtree
 * @param acc an optional value of the accumulated value
 * @param decider the predicate for suffix result
 * @return the inclusive start of the smallest suffix that satisfies the given condition
 */
template<typename F, typename T, typename Tree>
std::size_t suffix_search_helper(Tree& tree, T* curr, std::size_t left, std::size_t right, std::optional<T> acc, F& decider) {
    if (right - left == 1) {
        return left;
    }
    if constexpr (std::is_void_v<typename segment_tree_decidier<Tree>::type>) {
        (const_cast<Tree&>(tree)).push(curr, left, right);
    }
    T* left_root = curr + 1;
    std::size_t mid = (right - left) / 2 + left;
    T* right_root = curr + 2 * (mid - left);
    if ((acc && decider(tree.op(*right_root, *acc))) || decider(*right_root)) {
        return suffix_search_helper(tree, right_root, mid, right, try_move(acc), decider);
    }
    std::optional<T> new_acc = std::make_optional(acc ? tree.op(*right_root, *acc) : *right_root);
    return suffix_search_helper(tree, left_root, left, mid, try_move(new_acc), decider);
}

/**
 * @brief Get the index of the inclusive start of the smallest suffix that satisfies the given condition in a range
 * 
 * Runs in log(n) time where n is the size of the segment tree
 * 
 * @pre decider must meet the requirement that decider(op(data[i:])) implies decider(op(data[j:])) where i > j
 *      Most common operations such as addition, min and max all satisfy this requirement.
 * @tparam F the type of the predicate for suffix result
 * @param decider the predicate for suffix result
 * @param first the inclusive begin of the range
 * @param last the exclusive end of the range
 * @return the index if such suffix of the given range exists, or else an empty optional
 */
template<typename Tree, typename F>
std::optional<std::size_t> suffix_search(Tree& tree, F& decider, std::size_t first, std::size_t last) {
    if (first == last) {
        std::optional<std::size_t>();
    }
    typename segment_tree_traits<Tree>::subtree_unique_ptr subtrees = collect_subtrees(tree, first, last);
    std::size_t subtree_count = subtrees.get_deleter().size();
    using T = typename Tree::value_type;
    std::optional<T> acc;
    for (int i = subtree_count - 1; i >= 0; --i) {
        std::optional<T> new_acc(acc ? tree.op(*std::get<0>(subtrees[i]), acc.value()) : *std::get<0>(subtrees[i]));
        if (decider(new_acc.value())) {
            return std::make_optional(suffix_search_helper(tree, std::get<0>(subtrees[i]), std::get<1>(subtrees[i]), 
                std::get<2>(subtrees[i]), try_move(acc), decider));
        }
        acc = try_move(new_acc);
    }
    return std::optional<std::size_t>();
}

template<typename T>
union value_holder {
    value_holder() {}

    ~value_holder() {}
    bool dummy;
    T val;
};
}