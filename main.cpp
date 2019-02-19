#include "gtest/gtest.h"
#include "AdjacencyMatrix.h"
#include "LargeGraphTest.cpp"
#include "AdjacencyLists.h"
#include "DKTree.h"

void doLargeTests();

int main(int argc, char **argv) {
//    doLargeTests();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void doLargeTests() {
    unsigned long size = 10000;
    unsigned long m = size * 5;
    printf("Size: %lu, m: %lu\n", size, m);
//    printf("\nAdjacency matrix:\n");
//    largeTest<AdjacencyMatrix>(size, m);
    printf("\nAdjacency lists:\n");
    largeTest<AdjacencyLists>(size, m);
    printf("\ndk2tree:\n");
    auto tree = largeTest<DKTree>(size, m);
    tree->printtt();
}
