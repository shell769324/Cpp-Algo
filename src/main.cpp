#include <vector>
#include "vector.h"
#include "tree/binary_tree_common.h"
#include "tree/avl_tree.h"
#include "experimental/base2.h"

int main() {
    int_iterator<> it(4);
    std::cout << it.incr() << " ";
    int_iterator<true> it2(4);
    std::cout << it2.incr() << " ";
}
