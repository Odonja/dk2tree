#include "gtest/gtest.h"
#include "AdjacencyMatrix.h"
#include "LargeGraphTest.cpp"
#include "AdjacencyLists.h"
#include "DKTree.h"

void doLargeTests();

int main(int argc, char **argv) {
    doLargeTests();
}

void doLargeTests() {
    unsigned long size = 10000;
    double p = 250.0 / size;
//    printf("\nAdjacency matrix:\n");
//    largeTest<AdjacencyMatrix>(size, p);
    printf("\nAdjacency lists:\n");
    largeTest<AdjacencyLists>(size, p);
    printf("\ndk2tree:\n");
    largeTest<DKTree>(size, p);
}
