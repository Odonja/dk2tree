//
// Created by hugo on 9-2-19.
//

#include <cstdio>
#include "gtest/gtest.h"
#include "AdjacencyLists.h"
#include <iostream>

TEST(AdjLists, EmptyGraph) {
    AdjacencyLists a;
    ASSERT_EQ(a.totalSize(), 0);
    ASSERT_EQ(a.usedSize(), 0);
    ASSERT_THROW(a.reportEdge(0, 1), invalid_argument);
    ASSERT_THROW(a.addEdge(1, 1), invalid_argument);
    ASSERT_THROW(a.removeEdge(1, 0), invalid_argument);
}

TEST(AdjLists, Graph1) {
    AdjacencyLists a;
    unsigned long pos = a.insertEntry();
    ASSERT_EQ(pos, 0);
    ASSERT_FALSE(a.reportEdge(0, 0));
    a.addEdge(0, 0);
    ASSERT_TRUE(a.reportEdge(0, 0));
    a.removeEdge(0, 0);
    ASSERT_FALSE(a.reportEdge(0, 0));
}

TEST(AdjLists, Graph2) {
    AdjacencyLists a;
    unsigned long x = a.insertEntry();
    unsigned long y = a.insertEntry();
    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 1);
    a.addEdge(0, 1);
    ASSERT_TRUE(a.reportEdge(0, 1));
    ASSERT_FALSE(a.reportEdge(1, 0));
    ASSERT_FALSE(a.reportEdge(0, 0));
    ASSERT_FALSE(a.reportEdge(1, 1));
    ASSERT_THROW(a.reportEdge(0, 2), invalid_argument);
}

TEST(AdjLists, LargeGraph) {
    unsigned long size = 250;
    AdjacencyLists a(size);
    ASSERT_EQ(a.totalSize(), size);
    ASSERT_EQ(a.usedSize(), size);
    for (unsigned long i = 0; i < size; i++) {
        for (unsigned long j = 0; i < size; i++) {
            ASSERT_FALSE(a.reportEdge(i, j));
        }
    }
    a.addEdge(       0,        0);
    a.addEdge(       0, size - 1);
    a.addEdge(size - 1,        0);
    a.addEdge(size - 1, size - 1);
    for (unsigned long i = 0; i < size; i++) {
        bool edgeI = (i == 0 || i == size - 1);
        for (unsigned long j = 0; j < size; j++) {
            bool edgeJ = (j == 0 || j == size - 1);
            bool edge = (edgeI && edgeJ);
            ASSERT_EQ(edge, a.reportEdge(i, j));
        }
    }
    ASSERT_THROW(a.addEdge(0, size), invalid_argument);
    ASSERT_THROW(a.addEdge(size, 0), invalid_argument);
}

TEST(AdjLists, LargeGraphWithRemovals) {
    unsigned long size = 250;
    unsigned long toDelete = 123;
    AdjacencyLists a(size);
    a.addEdge(0, 0);
    a.addEdge(0, toDelete);
    a.addEdge(toDelete, 0);
    a.addEdge(toDelete, toDelete);

    a.deleteEntry(toDelete);

    ASSERT_EQ(a.totalSize(), size);
    ASSERT_EQ(a.usedSize(), size - 1);

    ASSERT_TRUE(a.reportEdge(0, 0));
    ASSERT_THROW(a.reportEdge(0, toDelete), invalid_argument);
    ASSERT_THROW(a.reportEdge(toDelete, 0), invalid_argument);
    ASSERT_THROW(a.reportEdge(toDelete, toDelete), invalid_argument);

    unsigned long pos = a.insertEntry();

    ASSERT_EQ(pos, toDelete);

    ASSERT_TRUE(a.reportEdge(0, 0));
    ASSERT_FALSE(a.reportEdge(0, toDelete));
    ASSERT_FALSE(a.reportEdge(toDelete, 0));
    ASSERT_FALSE(a.reportEdge(toDelete, toDelete));
}
