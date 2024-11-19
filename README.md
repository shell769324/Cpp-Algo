# CPP Algo
Generic C++ containers and algorithms

## Overview
This project provides robust C++ implementation for common and advanced data structures and algorithms. The implementations leverage features from C++11, C++20
such as **auto, move semantics, perfect forwarding, concepts** in conjunction with template, constexpr etc. Interfaces and implementation strictly follow the requirements in the C++20 standard.


## Testing strategy
### Correctness
To validate that the code behaves according to the C++20 standard, extensive unit tests and stress tests are written. To name a few:
* According to 22.2.1.3, node-based containers should instantiate the value type with the associated container by treating it as a buffer. All node-based containers in this library implement this
  requirement by abandoning the typical constructor/destructor paradigm and going with explicit object lifecycle management. Tests are done to ensure all objects' lifecycle are managed
  explicitly, bypassing direct constructor/destructor invocations.
* According to 22.3.8.4.2, exceptions thrown by memory allocation should make insert or push operations noops for a deque implementation. To implement this guarantee, memory allocation is done
  before elements are copied or moved. Any memory allocation exceptions are followed by a rollback that restores the deque to a valid state with no content change. Explicit tests are done by simulating
  a bad_alloc exception and the deque is validated after the rollback
* Additional self-imposed conditions are also tested. For example, red black tree should never reach depth 2logN and avl tree should never reach depth sqrt(2)logN.

### Time complexity guarantee
Critical operations (like push/pop for contiguous containers, find/merge for tree-based data structure) are timed. Results are used to fit a function of time datapoints
with the input size as the variable. The R2 score is calculated to validate the time complexity characteristics.

### Memory safety
There are two sources of memory leak, one from mismanagement of object lifecycle, and the other from mismamangement of raw memory. A testing harness is made to track how many 
objects are instantiated and destroyed, as well as how many of them are created on heap and freed. The testing harness uses mock objects and a tracking custom memory allocator to track
both. All unit tests are hooked to the harness to ensure completeness.


## Project Structure
The project is built using CMake 3.30. Implementations go into `src` directory and unit tests go into `tst` directory

## Roadmap
| Milestone       |  Description | Status|
|------------|-------------|-------------|
| Vector and stack | Generic vector and stack with strong exception guarantee and memory optimization | Complete |
| AVL tree | Generic AVL tree and an ordered map and ordered set implemented with it | Complete |
| Red black tree | Generic red black tree and an ordered map and ordered set implemented with it | Complete |
| Parallelism infra and binary tree parallelism | Thread pool executor implementation and parallel implementation of union, intersection and difference | Complete |
| Deque | A generic deque with better memory characteristics than gcc implementation | Complete |
| Binary indexed tree and Segment tree | Generic binary indexed tree, segment tree and range segment tree based on lazy propagation | Complete |
| Custom allocator | Custom allocator support for all data structures and add test harness to trace memory leak | Complete |
| Chunk allocator | Parallel chunk allocator that saves raw memory allocation | Milestone 8 |
| Splay tree | A generic tree data structure that supports fast access for recently accessed elements | Milestone 9 |
| Treap with augmentation | A probabilistic tree data structure that supports associative operator| Milestone 10 |
| Performance tuning | Performance Benchmark for all tree implementations and memory allocation improvement | Milestone 11 |


## Testing
All implementations have complete unit test suite as well as stress tests to ensure correctness. Properties of data structures and memory allocation aspect are also tested.
Performance benchmark will be added later.

To run the test, first create and `cd` iinto a build folder in the root directory. Then run 
`cmake .. && make all`. Then you can run `./tst/cpp_algo_tst` to run the tests.
