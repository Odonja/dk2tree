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
        DKTree dktree;
        dktree.printtt();
        // ttree vd dktree is een leaf node met k^2 nullen
        BitVector b(3);
        b.set(2, true);
        ASSERT_EQ(1, b.rank1(3));
    }

    TEST(DKTreeTest, insertOneEntry) {
        DKTree dktree;
        unsigned long position = dktree.insertEntry();
        dktree.printtt();
        ASSERT_EQ(0, position);
        // ttree vd dktree is een leaf node met k^2 nullen
    }

    TEST(DKTreeTest, insertTwoEntries) {
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        dktree.printtt();
        ASSERT_NE(positionA, positionB);
        // a en b hebben een andere positie
    }

    TEST(DKTreeTest, findNoEdge) {
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        dktree.printtt();
        bool resultAtoB = dktree.reportEdge(positionA, positionB);
        bool resultBtoA = dktree.reportEdge(positionB, positionA);
        ASSERT_FALSE(resultAtoB);
        ASSERT_FALSE(resultBtoA);
    }

}
#pragma clang diagnostic pop