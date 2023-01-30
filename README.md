<<<<<<< HEAD
# Cpp Algo
=======
# CPP Algo
>>>>>>> 82acec1 (1. Add deque and some unit tests for it)
Generic C++ containers and algorithms

## Overview
This project provides robust C++ implementation for common and advanced data structures and algorithms. The implementations leverage features from C++11, C++20
such as **auto, move semantics, perfect forwarding, concepts** in conjunction with template, constexpr etc.

## Project Structure
The project is built using CMake 3.21. Implementations go into `src` directory and unit tests go into `tst` directory

## Roadmap
| Milestone       |  Description | Status|
|------------|-------------|-------------|
| Vector and stack | Generic vector and stack with strong exception guarantee and memory optimization | Complete |
| AVL tree | Generic AVL tree and an ordered map and ordered set implemented with it | Complete |
| Red black tree | Generic red black tree and an ordered map and ordered set implemented with it | Complete |
| Parallelism infra and binary tree parallelism | Thread pool executor implementation and parallel implementation of union, intersection and difference | Complete |
| Deque | A generic deque with better memory characteristics | In Progress |
| Binary indexed tree and Segment tree | Generic binary indexed tree and segment tree | In Progress |
| Splay tree | A generic tree data structure that supports fast access for recently accessed elements | Milestone 7 |
| Treap with augmentation | A probabilistic tree data structure that supports associative operator| Milestone 8 |
| Performance tuning | Performance Benchmark for all tree implementations and memory allocation improvement | Milestone 9 |


## Testing
All implementations have complete unit test suite as well as stress tests to ensure correctness. Properties of data structures and memory allocation aspect are also tested.
Performance benchmark will be added later.

To run the test, first create and `cd` iinto a build folder in the root directory. Then run 
`cmake .. && make all`. Then you can run `./tst/cpp_algo_tst` to run the tests.