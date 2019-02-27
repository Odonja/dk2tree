#include "gtest/gtest.h"

#include "LargeGraphTest.cpp"

#include "AdjacencyLists.h"
#include "AdjacencyMatrix.h"
#include "DKTree.cpp"

#include "AdjacencyListsTest.cpp"
#include "AdjacencyMatrixTest.cpp"
#include "BitVectorTest.cpp"
#include "DKTreeTest.cpp"
#include "TTreeTest.cpp"

void doLargeTests();

int main(int argc, char **argv) {
//    doLargeTests();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void doLargeTests() {
    std::cout << ::getpid() << std::endl;

    unsigned long size = 10000;
    unsigned long m = size * 5;
    printf("Size: %lu, m: %lu\n", size, m);

//    printf("\nAdjacency matrix:\n");
//    auto matrix = largeTest<AdjacencyMatrix>(size, m);
//    delete matrix;

//    printf("\nAdjacency lists:\n");
//    auto graph = largeTest<AdjacencyLists>(size, m);
//    delete graph;

    printf("\ndk2tree:\n");
    auto tree = largeTest<DKTree>(size, m);
    delete tree;
}
