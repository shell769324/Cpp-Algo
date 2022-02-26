#include "gtest/gtest.h"

int main(int argc, char **argv) {
    std::srand(7759);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
