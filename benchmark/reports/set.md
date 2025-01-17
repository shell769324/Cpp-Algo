# Set benchmark
Four types of elements are tested. They are described below. Both my and gcc sets are compiled with `Release` which uses O3 optimization.
Due to the unpredictable nature of tree data structures, I defined 18 (a random number I picked) different orders to insert the same values.
This eliminates the advantage of certain implementation seen in a particular favorable insertion order.

| element type     | size in bytes | description                                   | 
|------------------|---------------|-----------------------------------------------|
| `int`            | 8             | N/A                                           |
| `small_element`  | 8             | contains a single long                        |
| `medium_element` | 58            | contains a long and a char buffer of size 50  |
| `big_element`    | 508           | contains a long and a char buffer of size 500 |

## Constant read operations
![begin](../generated/set/begin.png)
![end](../generated/set/end.png)
![empty](../generated/set/empty.png)
![size](../generated/set/size.png)

These constant operations are tested by creating sets of different sizes and measure their performance through repeated invocations. 
My `begin()` is faster because `gcc` involves a few more pointer accesses which manifest as an extra `mov` execution when there is a scarcity
of registers. Similar explanation can be offered for the `empty()` and `size()` performance difference.

## Iteration
![iterate](../generated/set/iterate.png)

This benchmark uses a for loop to accumuate all values in the set. All implementations have the same performance.

## Lookup operations
![find](../generated/set/find.png)

![lower_bound](../generated/set/lower_bound.png)

![upper_bound](../generated/set/upper_bound.png)

All three methods are tested by mixing 50% of missing targets with 50% of present targets to simulate realistic use case. 
Both my avl and red black trees outperform gcc red black tree in most cases, but their overall performance is comparable. 
One thing to note is despite the popular belief that avl tree is more balanced than red black tree, their lookup performance 
don't show any visible difference as their input size scales.

## Insertion
![insert_absent](../generated/set/insert_absent.png)

![insert_present](../generated/set/insert_present.png)

Inserting new/existing elements have great performance difference since inserting new elements require memory allocation and rebalance.
I benchmarked both cases separately for maximum clarity. For the successful insertion benchmark, the set is initially empty and the
time to insert all values is measured. For the benchmark of inserting existing elements, the set is initialized with all values, and
the time to attempt to insert them is measured.

Both my avl and red black trees outperform gcc red black tree in most cases, but their overall performance is comparable.
One interesting finding is present/absent bigo coefficients only differ by a fraction, and the fraction is greater when the 
element type is greater. This suggests that rebalancing takes less time than the downward pass to find the insertion parent 
since it is dominated by memory allocation.

![insert_hint_greater_hint](../generated/set/insert_hint_greater_hint.png)

![insert_hint_smaller_hint](../generated/set/insert_hint_smaller_hint.png)

Another common insertion method is insertion with hint. This is useful when the elements to insert are already ordered, so we can give
a hint to the data structure about where the insertion position may be. There are two symmetric cases, one with the hint smaller
than the insertion position, and the other with the hint greater. In either cases, my red black tree impl is the fastest, and the gcc
red black tree and my avl tree have comparable performance. This echoes the common belief that red black tree has a faster rebalance
pass than avl tree since insertion with hint removes the pass to find insertino parent. Most time is spent in memory allocation and
rebalancing.

## Erasure
![erase_absent](../generated/set/erase_absent.png)

![erase_present](../generated/set/erase_present.png)

Like insertion, erasing absent/present elements have great performance difference since erasing present elements require memory deallocation and rebalance.
I benchmarked both cases separately for maximum clarity. For the successful erasure benchmark, the set is initialized and the time
to erase them all is measured. For the benchmark of erasing absent elements, the set is also initialized, but we only attempt to erase elements that
don't exist. Therefore the set size remains constant in the entire benchmark

My red black tree outperforms the other two sets in all cases, and the difference is greater when the element type is bigger.

## Bulk operations
My set operations additionally support join-based bulk operations which have a better time complexity than naive bulk operations. They 
can also be parallelized to further boost performance. Given two sets of sizes `N` and `M` where `N <= M`, the time complexity for taking
the union of them is `O(Nlog(M/N + 1))`. This becomes `O(N)` when `N = M`, and `O(logM)` when `N = 1`. Since there are two variables in
the bigo notation, I performed two different benchmarks to observe the performance change along two dimensions. I first tested the case
when `N = M` and I scaled their value by a factor of 2. Then I tested the case when `M` stays at a big value, 2^17, and I scaled `N`
by a factor of 2. This same methodology is used for the other two bulk operations. To make a realistic example, I initialized the two sets
such that each element in a set has 1/2 chance to also appear in the other set.

To better showcase the superiority of join-based implementations, I also implemented the naive union, i.e. inserting all values in the smaller
set into the bigger one, to serve as a comparison.
![union_of_balanced](../generated/set/union_of_balanced.png)

![union_of_unbalanced](../generated/set/union_of_unbalanced.png)

Clearly the join based union scales better despite having a greater coefficient.

![intersection_of_balanced](../generated/set/intersection_of_balanced.png)

![intersection_of_unbalanced](../generated/set/intersection_of_unbalanced.png)

Like union, I also implemented naive intersection for gcc set by removing elements absent in the bigger set in the smaller set. An interesting
observation is while the thereotical time complexity for join-based intersection is `O(Nlog(M/N + 1))`, the actual time complexity 
is dominated by memory deallocation, especially when the intersection of two sets are small compared to the original sizes.

![difference_of_balanced](../generated/set/difference_of_balanced.png)

![difference_of_unbalanced](../generated/set/difference_of_unbalanced.png)

Like union, I also implemented naive difference for gcc set by removing all elements in one set from the other set.
