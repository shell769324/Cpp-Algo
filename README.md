# Star Anise
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
| AVL tree | Generic AVL tree that provides an easy ordered map and ordered set implementation | Complete |
| Red black tree | Generic red black tree that provides an easy ordered map and ordered set implementation | Next |
| Augmented tree plugin | A plugin class that allows augmentation for any binary trees | Milestone 4 |
| Splay tree |  | Milestone 5 |
| Treap | | Milestone 6 |
| Performance tuning | Performance Benchmark for all tree implementations and memory allocation improvement | Milestone 7 |


## Testing
All implementations have complete unit test suite as well as stress tests to ensure correctness. Properties of data structures and memory allocation aspect are also tested.
Performance benchmark will be added later.
