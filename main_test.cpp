#include "gtest/gtest.h"

#include "LargeGraphTest.cpp"
#include "MakeGraphFromFile.cpp"

#include "DKTree.cpp"

#include "BitVectorTest.cpp"
#include "DKTreeTest.cpp"
#include "TTreeTest.cpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
