# CPP Algo
Generic C++ containers and algorithms

## Overview
This project provides robust C++ implementation for common and advanced data structures and algorithms. The implementations leverage 
features from C++11, C++20 such as **auto, move semantics, perfect forwarding, concepts** in conjunction with template, constexpr etc.
Interfaces and implementations follow the requirements in the C++20 standard. Due to the scope of the standard, not all overloads
of all APIs are implemented. All API families introduced before or at C++14 have most overloads implemented.

## Why prefer this over gcc?
Some implementations in this project outperform the same data structure in gcc, and some have differently balanced characteristics than those in gcc. You can view the benchmark reports for details.
In addition, some data structures or APIs are only unique to this project. For example, this project provides templated binary indexed tree and some parallel algorithms 
on tree-based data structures. Those are unlikely to be included in the standard.

## Testing strategy
### Correctness
To validate that the code behaves according to the C++20 standard, extensive unit tests and stress tests are written. To name a few:
* According to 22.2.1.3, node-based containers should instantiate the value type with the associated container by treating it as a buffer.
  All node-based containers in this library implement this requirement by abandoning the typical constructor/destructor paradigm and 
  going with explicit object lifecycle management. Tests are done to ensure all objects' lifecycle are managed explicitly, bypassing 
  direct constructor/destructor invocations.
* According to 22.3.8.4.2, exceptions thrown by memory allocation for insert or push operations should have no effects for deque. 
  To implement this guarantee, memory allocation is done before elements are copied or moved. Any memory allocation exceptions are 
  followed by a rollback that restores the deque to a valid state with no content change. Explicit tests are done by simulating
  a bad_alloc exception and the deque is validated after the rollback.
* According to 22.3.11.3.9, shrink_to_fit should provide strong exception guarantee unless an exception is thrown by the move 
  constructor of a non-Cpp17CopyInsertable type. This is reflected in the implementation by using concept and type trait.
* According to 22.1.1.10, all containers' `begin()` and `end()` are constant. While this is unnatural for tree based containers, 
  implementations in this project meet this requirement by tracking the current least node in all mutating APIs.
* Additional self-imposed conditions are also tested. For example, red black tree should never reach depth 2logN and avl tree should never reach depth sqrt(2)logN.

### Time complexity guarantee
Critical operations (like push/pop for sequence containers, find/merge for tree-based containers) are benchmarked with google benchmark. 
Reports are generated as markdown files. You can view them in `benchmark/reports` directory.

### Memory safety
There are two sources of memory leak, one from mismanagement of object lifecycle, and the other from mismamangement of raw memory. 
A testing harness is made to track how many objects are instantiated and destroyed, as well as how many of them are created on heap and 
freed. The testing harness uses mock objects and a tracking custom memory allocator to track both. All unit tests are hooked to the 
harness to ensure completeness.

### Run tests
To run the test,
* Create and `cd` into a build folder in the root directory
* Run `cmake ..` to generate a build system. You only need to run it once unless you make changes to the `CMakeLists.txt` file.
* run `cmake --build . --target cpp_algo_tst` to build test code.
* Run `./tst/cpp_algo_tst` to run the tests. You may run `./tst/cpp_algo_tst --gtest_filter=vector_*` for example to
  only run vector unit tests.

### Run benchmarks
To run a specific benchmark, do the following steps
* Create and `cd` into a build folder in the root directory.
* Run `cmake ..` to generate a build system. You only need to run it once unless you make changes to the `CMakeLists.txt` file.
* Run `cmake --build . --target cpp_algo_benchmark` to build benchmark code.
* Go back to the root of this project and run `python -m venv ./.venv` to create a python environment. This only needs to be run once.
* Run the following to benchmark this data structure family. A data structure family may include my implementation(s) and/or gcc implementation.
  This will produce a json file at `benchmark/generated/$data_structure_family.json` which can be then used to generate report with graphs.
```
data_structure_family=
./.venv/bin/python3 ./benchmark/run_benchmark.py $data_structure_family
```
To see the list of all available data structure families, run `./.venv/bin/python3 ./benchmark/run_benchmark.py --help`.
* Note that google benchmark runs many iterations of the same test to guarantee statistical stability, so running all benchmarks may be slow. 
  You can pass a regex to limit the benchmarks to run. For example, you can specify 
`./.venv/bin/python3 ./benchmark/run_benchmark.py deque -r "deque_push_back*"` to run only `push_back` benchmark.
* To generate the report, run `./.venv/bin/python3 ./benchmark/generate_report.py "$data_structure_family"`. These will populate the performance plots. 
  You can then view the markdown report file in the `./benchmark/reports/$data_structure_family` directory.

## Performance tuning
This project pays heavy attention to performance and the goal is to use gcc implementations as a baseline for the same data structure. 
Some performance tricks are used, such as segment iterator optimization for deque, high order bit to store boolean flag for red black 
tree, code that encourages cmov over branching (when cmov is measured to be faster). Move operations are preferred when they don't 
violate standards. Trivial object types are also exploited to avoid individual assignment operator invocations.

## Project Structure
The project is built using CMake 3.30. Implementations go into the `src` directory, unit tests go into the `tst` directory and benchmarks go into the `benchmark` directory.

## Roadmap
| Milestone               |  Description                                                                                          | Status       |
|-------------------------|-------------------------------------------------------------------------------------------------------|--------------|
| Vector and stack        | Generic vector and stack with strong exception guarantee and memory optimization                      | Complete     |
| AVL tree                | Generic AVL tree and an ordered map and ordered set implemented with it                               | Complete     |
| Red black tree          | Generic red black tree and an ordered map and ordered set implemented with it                         | Complete     |
| Binary tree parallelism | Thread pool executor implementation and parallel implementation of union, intersection and difference | Complete     |
| Deque                   | A generic deque with better memory characteristics than gcc implementation                            | Complete     |
| Range query trees       | Generic binary indexed tree, segment tree and range segment tree based on lazy propagation            | Complete     |
| Custom allocator        | Custom allocator support for all data structures and add test harness to trace memory leak            | Complete     |
| Performance tuning      | Performance Benchmark for all containers and memory allocation improvements                           | Complete     |
| Chunk allocator         | Parallel chunk allocator that saves raw memory allocation                                             | Milestone 9  |
| Splay tree              | A generic tree data structure that supports fast access for recently accessed elements                | Milestone 10 |
| Treap with augmentation | A probabilistic tree data structure that supports associative operator                                | Milestone 11 |
