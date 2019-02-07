#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-equals-delete"
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by anneke on 05/02/19.
//

#include "DKTree.h"
#include <cstdio>
#include "gtest/gtest.h"
#include <iostream>

using namespace std;

namespace {

    TEST(DKTreeTest, emptyTree) {
        std::cout << "emptyTree test\n";
        DKTree dktree;
        dktree.printtt();
        // ttree vd dktree is een leaf node met k^2 nullen
        BitVector b(3);
        b.set(2, true);
        ASSERT_EQ(1, b.rank1(3));
    }

    TEST(DKTreeTest, insertOneEntry) {
        std::cout << "insert One Entry test\n";
        DKTree dktree;
        unsigned long position = dktree.insertEntry();
        dktree.printtt();
        ASSERT_EQ(0, position);
        // ttree vd dktree is een leaf node met k^2 nullen
    }

    TEST(DKTreeTest, insertTwoEntries) {
        std::cout << "insert two entries test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        dktree.printtt();
        ASSERT_NE(positionA, positionB);
        // a en b hebben een andere positie
    }

    TEST(DKTreeTest, findNoEdge) {
        std::cout << "find no edge test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        dktree.printtt();
        bool resultAtoB = dktree.reportEdge(positionA, positionB);
        bool resultBtoA = dktree.reportEdge(positionB, positionA);
        ASSERT_FALSE(resultAtoB);
        ASSERT_FALSE(resultBtoA);
    }

    TEST(DKTreeTest, errorShouldBeThrown) {
        std::cout << "error should be thrown test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        bool resultAtoB = dktree.reportEdge(positionA, positionB);
        ASSERT_FALSE(resultAtoB);
        unsigned long unoccupiedPosition = positionA + positionB+1;

        try {
            dktree.reportEdge(positionA, unoccupiedPosition);
            ASSERT_FALSE(true); // should not be reached
        } catch (const std::invalid_argument& e) {
            std::stringstream error;
            error << "reportEdge: invalid argument " << unoccupiedPosition << ", position not occupied in matrix\n";

            ASSERT_EQ(error.str(), e.what());
        }
    }

    TEST(DKTreeTest, find1PosEdge) {
        std::cout << "find edge test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        try {
            dktree.addEdge(positionA, positionB);
        }catch (...){
            std::cout <<"--------------------problems in find edge test\n";
        }


        dktree.printtt();
        bool resultAtoB = dktree.reportEdge(positionA, positionB);
        bool resultBtoA = dktree.reportEdge(positionB, positionA);
        ASSERT_TRUE(resultAtoB);
        ASSERT_FALSE(resultBtoA);
    }

    TEST(DKTreeTest, findSecondBlockPosEdge) {
        std::cout << "findSecondBlockPosEdge test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
         dktree.insertEntry();
         dktree.insertEntry();
         dktree.insertEntry();
        unsigned long positionE = dktree.insertEntry();
        try {
            dktree.addEdge(positionE, positionE);
        }catch (...){
            std::cout <<"--------------------problems in find edge test\n";
        }

        dktree.printtt();
        bool resultAtoB = dktree.reportEdge(positionA, positionE);
        bool resultBtoA = dktree.reportEdge(positionE, positionE);
        ASSERT_TRUE(resultBtoA);
        ASSERT_FALSE(resultAtoB);
    }

    TEST(DKTreeTest, findOuterCorners) {
        std::cout << "find edge test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        unsigned long positionB = 15;
        for(unsigned long i = 0; i < 16; i++){
            dktree.insertEntry();
        }
        dktree.addEdge(positionA, positionA);
        dktree.addEdge(positionA, positionB);
        dktree.addEdge(positionB, positionA);
        dktree.addEdge(positionB, positionB);

        dktree.printtt();
        bool topLeft = dktree.reportEdge(positionA, positionA);
        bool topRight = dktree.reportEdge(positionA, positionB);
        bool bottomLeft = dktree.reportEdge(positionB, positionA);
        bool bottomRight = dktree.reportEdge(positionB, positionA);
        ASSERT_TRUE(topLeft);
        ASSERT_TRUE(topRight);
        ASSERT_TRUE(bottomLeft);
        ASSERT_TRUE(bottomRight);
    }

    TEST(DKTreeTest, findCenter) {
        std::cout << "find edge test\n";
        DKTree dktree;
        unsigned long positionA = 7;
        unsigned long positionB = 8;
        for(unsigned long i = 0; i < 16; i++){
            dktree.insertEntry();
        }
        dktree.addEdge(positionA, positionA);
        dktree.addEdge(positionA, positionB);
        dktree.addEdge(positionB, positionA);
        dktree.addEdge(positionB, positionB);

        dktree.printtt();
        bool centerTopLeft = dktree.reportEdge(positionA, positionA);
        bool centerTopRight = dktree.reportEdge(positionA, positionB);
        bool centerBottomLeft = dktree.reportEdge(positionB, positionA);
        bool centerBottomRight = dktree.reportEdge(positionB, positionA);
        ASSERT_TRUE(centerTopLeft);
        ASSERT_TRUE(centerTopRight);
        ASSERT_TRUE(centerBottomLeft);
        ASSERT_TRUE(centerBottomRight);
    }

    TEST(DKTreeTest, increaseTableSizeAfterAddingAnEdge) {
        std::cout << "find edge test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        unsigned long positionB = 31;
        for(unsigned long i = 0; i < 16; i++){
            dktree.insertEntry();
        }
        dktree.addEdge(positionA, positionA);
        for(unsigned long i = 0; i < 16; i++){
            dktree.insertEntry();
        }

        dktree.addEdge(positionA, positionB);
        dktree.addEdge(positionB, positionA);
        dktree.addEdge(positionB, positionB);

        dktree.printtt();
        bool centerTopLeft = dktree.reportEdge(positionA, positionA);
        bool centerTopRight = dktree.reportEdge(positionA, positionB);
        bool centerBottomLeft = dktree.reportEdge(positionB, positionA);
        bool centerBottomRight = dktree.reportEdge(positionB, positionA);
        ASSERT_TRUE(centerTopLeft);
        ASSERT_TRUE(centerTopRight);
        ASSERT_TRUE(centerBottomLeft);
        ASSERT_TRUE(centerBottomRight);
    }



}
#pragma clang diagnostic pop