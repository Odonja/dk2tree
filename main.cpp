#include "gtest/gtest.h"

#include "LargeGraphTest.cpp"
#include "MakeGraphFromFile.cpp"

#include "AdjacencyLists.h"
#include "AdjacencyMatrix.h"
#include "DKTree.cpp"

#include "AdjacencyListsTest.cpp"
#include "AdjacencyMatrixTest.cpp"
#include "BitVectorTest.cpp"
#include "DKTreeTest.cpp"
#include "TTreeTest.cpp"

void doLargeTests(bool);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int k = RUN_ALL_TESTS();
    if (k != 0) {
        return k;
    }
//    doLargeTests(true);
    auto tree = makeGraphFromFile("datasets/eu-2005.txt", true);
    std::cout << tree->memoryUsage() << std::endl;
}

void doLargeTests(bool verbose) {
    std::cout << ::getpid() << std::endl;

    unsigned long size = 100000;
    unsigned long m = size * 50;
    printf("Size: %lu, m: %lu\n", size, m);

//    printf("\nAdjacency matrix:\n");
//    auto matrix = largeTest<AdjacencyMatrix>(size, m, verbose);
//    delete matrix;

//    printf("\nAdjacency lists:\n");
//    auto graph = largeTest<AdjacencyLists>(size, m, verbose);
//    delete graph;

    printf("\ndk2tree:\n");
    auto tree = largeTest<DKTree>(size, m, verbose);
    delete tree;
}
