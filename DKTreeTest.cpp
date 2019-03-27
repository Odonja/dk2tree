#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-equals-delete"
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by anneke on 05/02/19.
//
#include <cstdio>
#include <iostream>
#include "gtest/gtest.h"
#include "stdlib.h"
#include "DKTree.h"

using namespace std;

namespace {

    TEST(DKTreeTest, emptyTree) {
        std::cout << "emptyTree test\n";
        DKTree dktree;
        dktree.printtt();
        // ttree of the dktree is a leaf node with k^2 zeroes
        BitVector<> b(3);
        b.set(2, true);
        ASSERT_EQ(1, b.rank1(3));
    }

    TEST(DKTreeTest, insertOneEntry) {
        std::cout << "insert One Entry test\n";
        DKTree dktree;
        unsigned long position = dktree.insertEntry();
        dktree.printtt();
        ASSERT_EQ(0, position);
        // ttree of the dktree is a leaf node with k^2 zeroes
    }

    TEST(DKTreeTest, insertTwoEntries) {
        std::cout << "insert two entries test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        dktree.printtt();
        ASSERT_NE(positionA, positionB);
        // a and b must have different positions
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
        unsigned long unoccupiedPosition = positionA + positionB + 1;

        try {
            dktree.reportEdge(positionA, unoccupiedPosition);
            ASSERT_FALSE(true); // should not be reached
        } catch (const std::invalid_argument &e) { }
    }

    TEST(DKTreeTest, find1PosEdge) {
        std::cout << " find1PosEdge test\n";
        DKTree dktree;
        unsigned long positionA = dktree.insertEntry();
        unsigned long positionB = dktree.insertEntry();
        try {
            dktree.addEdge(positionA, positionB);
        } catch (...) {
            std::cout << "--------------------problems in find edge test\n";
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
        } catch (...) {
            std::cout << "--------------------problems in find edge test\n";
        }

        dktree.printtt();
        bool resultAtoB = dktree.reportEdge(positionA, positionE);
        bool resultBtoA = dktree.reportEdge(positionE, positionE);
        ASSERT_TRUE(resultBtoA);
        ASSERT_FALSE(resultAtoB);
    }

    TEST(DKTreeTest, findOuterCorners) {
        std::cout << "findOuterCorners test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        unsigned long positionB = 15;
        for (unsigned long i = 0; i < 16; i++) {
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
        std::cout << "findCenter test\n";
        DKTree dktree;
        unsigned long positionA = 7;
        unsigned long positionB = 8;
        for (unsigned long i = 0; i < 16; i++) {
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
        std::cout << "increaseTableSizeAfterAddingAnEdge test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        unsigned long positionB = 31;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
        }
        dktree.addEdge(positionA, positionA);
        for (unsigned long i = 0; i < 16; i++) {
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

    TEST(DKTreeTest, addAndDeleteLeftCornerEdge) {
        std::cout << "addAndDeleteLeftCornerEdge test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        dktree.insertEntry();
        dktree.addEdge(positionA, positionA);
        dktree.removeEdge(positionA, positionA);
        dktree.printtt();
        bool topLeft = dktree.reportEdge(positionA, positionA);
        ASSERT_FALSE(topLeft);
    }

    TEST(DKTreeTest, addAndDeleteSingleEntry) {
        std::cout << "addAndDeleteSingleEntry test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        dktree.insertEntry();
        dktree.addEdge(positionA, positionA);
        dktree.deleteEntry(positionA);
        dktree.printtt();
        //bool topLeft = dktree.reportEdge(positionA, positionA);
        //ASSERT_FALSE(topLeft);
    }

    TEST(DKTreeTest, addAndDeleteLeftmiddleEdgeOthersStayTrue) {
        std::cout << "addAndDeleteLeftmiddleEdgeOthersStayTrue test\n";
        DKTree dktree;
        unsigned long positionA = 0;
        unsigned long positionB = 8;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
            dktree.addEdge(positionA, i);
        }
        dktree.removeEdge(positionA, positionB);
        dktree.printtt();
        for (unsigned long i = 0; i < 16; i++) {
            bool current = dktree.reportEdge(positionA, i);
            if (i == 8) {
                ASSERT_FALSE(current);
            } else {
                ASSERT_TRUE(current);
            }
        }
    }


    TEST(DKTreeTest, LargeRandomTest) {
        unsigned long n = 100;
        unsigned long m = 5000;
        DKTree tree;
        for (unsigned long i = 0; i < n; i++) {
            unsigned long pos = tree.insertEntry();
            ASSERT_EQ(pos, i);
        }
        unsigned long step = n * n / m;
        for (unsigned long i = 0; i < n * n; i += step) {
            unsigned long c = i % n;
            unsigned long r = i / n;
            tree.addEdge(r, c);
        }

        printf("size: %lu\n", tree.memoryUsage());

        for (unsigned long i = 0; i < n * n; i++) {
            bool exists = (i % step == 0);
            unsigned long c = i % n;
            unsigned long r = i / n;
            bool result = tree.reportEdge(r, c);
            ASSERT_EQ(exists, result);
            tree.removeEdge(r, c);
        }
        tree.printtt();
    }

    TEST(DKTreeTest, reportAllEdgesEmptyMatrixNoEdgesException) {
        DKTree tree;
        vector<unsigned long> rows;
        vector<unsigned long> columns;
        try {
            vector<std::pair<unsigned long, unsigned long>> findings = tree.reportAllEdges(rows, columns);
            ASSERT_FALSE(true); // should not be reached
        } catch (const std::invalid_argument &e) {
            std::stringstream error;
            error << "sortAndCheckVector: invalid argument, empty input vector \n";

            ASSERT_EQ(error.str(), e.what());
        }
    }

    TEST(DKTreeTest, reportAllEdges1Edge) {
        DKTree tree;
        tree.insertEntry();
        tree.addEdge(0, 0);
        vector<unsigned long> rows{0};
        vector<unsigned long> columns{0};
        vector<std::pair<unsigned long, unsigned long>> findings = tree.reportAllEdges(rows, columns);
        ASSERT_FALSE(findings.empty());
        ASSERT_EQ(0, findings[0].first);
        ASSERT_EQ(0, findings[0].second);
    }

    TEST(DKTreeTest, reportAllEdgesOneAAllB) {
        std::cout << "addAndDeleteLeftmiddleEdgeOthersStayTrue test\n";
        DKTree dktree;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
        }
        dktree.addEdge(6, 2);
        dktree.addEdge(6, 5);
        dktree.addEdge(6, 8);
        dktree.addEdge(6, 13);
        ASSERT_TRUE(dktree.reportEdge(6, 2));
        ASSERT_TRUE(dktree.reportEdge(6, 5));
        ASSERT_TRUE(dktree.reportEdge(6, 8));
        ASSERT_TRUE(dktree.reportEdge(6, 13));
        dktree.printtt();
        vector<unsigned long> rows{6};
        vector<unsigned long> columns{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        vector<std::pair<unsigned long, unsigned long>> findings = dktree.reportAllEdges(rows, columns);
        for (auto finding:findings) {
            std::cout << "<" << finding.first << ", " << finding.second << ">, ";
        }
        printf("\n");
        ASSERT_EQ(4, findings.size());
        ASSERT_EQ(6, findings[0].first);
        ASSERT_EQ(2, findings[0].second);
        ASSERT_EQ(6, findings[1].first);
        ASSERT_EQ(5, findings[1].second);
        ASSERT_EQ(6, findings[2].first);
        ASSERT_EQ(8, findings[2].second);
        ASSERT_EQ(6, findings[3].first);
        ASSERT_EQ(13, findings[3].second);
    }

    TEST(DKTreeTest, reportAllEdgesOneBAllA) {
        std::cout << "addAndDeleteLeftmiddleEdgeOthersStayTrue test\n";
        DKTree dktree;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
        }
        dktree.addEdge(2, 6);
        dktree.addEdge(5, 6);
        dktree.addEdge(8, 6);
        dktree.addEdge(13, 6);
        ASSERT_TRUE(dktree.reportEdge(2, 6));
        ASSERT_TRUE(dktree.reportEdge(5, 6));
        ASSERT_TRUE(dktree.reportEdge(8, 6));
        ASSERT_TRUE(dktree.reportEdge(13, 6));
        dktree.printtt();
        vector<unsigned long> columns{6};
        vector<unsigned long> rows{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        vector<std::pair<unsigned long, unsigned long>> findings = dktree.reportAllEdges(rows, columns);
        for (auto finding:findings) {
            std::cout << "<" << finding.first << ", " << finding.second << ">, ";
        }
        printf("\n");
        ASSERT_EQ(4, findings.size());
        ASSERT_EQ(2, findings[0].first);
        ASSERT_EQ(6, findings[0].second);
        ASSERT_EQ(5, findings[1].first);
        ASSERT_EQ(6, findings[1].second);
        ASSERT_EQ(8, findings[2].first);
        ASSERT_EQ(6, findings[2].second);
        ASSERT_EQ(13, findings[3].first);
        ASSERT_EQ(6, findings[3].second);


    }

    TEST(DKTreeTest, testSorting) {
        vector<unsigned long> v{1, 5, 8, 9, 6, 7, 3, 4, 2, 0, 2, 2, 2};

        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
        v.erase(v.begin() + 3);


        cout << "Sorted \n";
        for (auto x : v) {
            cout << x << " ";
        }
        ASSERT_EQ(9, v.size());
    }

    TEST(DKTreeTest, addAndDeleteLeftmiddleEdgesOthersStayTrue) {
        std::cout << "addAndDeleteLeftmiddleEdgesOthersStayTrue test\n";
        DKTree dktree;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
        }
        dktree.addEdge(1, 2);
        dktree.addEdge(1, 3);
        dktree.addEdge(1, 4);
        dktree.addEdge(2, 9);
        dktree.addEdge(3, 0);
        dktree.addEdge(3, 1);
        dktree.addEdge(3, 6);
        dktree.addEdge(5, 7);
        dktree.addEdge(5, 8);
        dktree.addEdge(6, 2);
        dktree.addEdge(6, 5);
        dktree.addEdge(6, 8);
        dktree.addEdge(6, 13);
        dktree.addEdge(7, 6);
        dktree.addEdge(8, 6);
        dktree.addEdge(9, 6);
        dktree.addEdge(11, 4);
        dktree.addEdge(13, 6);
        dktree.addEdge(15, 6);
        dktree.deleteEntry(6);
        dktree.printtt();

        vector<unsigned long> validEntries{0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        vector<std::pair<unsigned long, unsigned long>> findings = dktree.reportAllEdges(validEntries, validEntries);
        for (auto finding:findings) {
            std::cout << "<" << finding.first << ", " << finding.second << ">, ";
        }
        printf("\n");
        ASSERT_EQ(9, findings.size());
        ASSERT_EQ(1, findings[0].first);
        ASSERT_EQ(2, findings[0].second);
        ASSERT_EQ(1, findings[1].first);
        ASSERT_EQ(3, findings[1].second);
        ASSERT_EQ(3, findings[2].first);
        ASSERT_EQ(0, findings[2].second);
        ASSERT_EQ(3, findings[3].first);
        ASSERT_EQ(1, findings[3].second);
        ASSERT_EQ(1, findings[4].first);
        ASSERT_EQ(4, findings[4].second);
        ASSERT_EQ(5, findings[5].first);
        ASSERT_EQ(7, findings[5].second);
        ASSERT_EQ(2, findings[6].first);
        ASSERT_EQ(9, findings[6].second);
        ASSERT_EQ(5, findings[7].first);
        ASSERT_EQ(8, findings[7].second);
        ASSERT_EQ(11, findings[8].first);
        ASSERT_EQ(4, findings[8].second);

    }

    TEST(DKTreeTest, addEdgeAfterDeletingEdgeGivesDeletedEdge) {
        std::cout << "addEdgeAfterDeletingEdgeGivesDeletedEdge test\n";
        DKTree dktree;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
        }
        dktree.deleteEntry(6);
        unsigned long newEntry = dktree.insertEntry();
        ASSERT_EQ(6, newEntry);
    }

    TEST(DKTreeTest, addEdgeAfterDeletingMultipleEdgesGivesDeletedEdgeWithLowestIndex) {
        std::cout << "addEdgeAfterDeletingMultipleEdgesGivesDeletedEdgeWithLowestIndex test\n";
        DKTree dktree;
        for (unsigned long i = 0; i < 16; i++) {
            dktree.insertEntry();
        }
        dktree.deleteEntry(6);
        dktree.deleteEntry(4);
        unsigned long newEntry = dktree.insertEntry();
        ASSERT_EQ(4, newEntry);
    }

    TEST(DKTreeTest, testpairsort){
        vector<std::pair<unsigned long, unsigned long>> allEdges;
        allEdges.emplace_back(6, 6);
        allEdges.emplace_back(6, 2);
        allEdges.emplace_back(4, 8);
        allEdges.emplace_back(4, 9);
        sort(allEdges.begin(), allEdges.end());
        for (auto finding:allEdges) {
            std::cout << "<" << finding.first << ", " << finding.second << ">, ";
        }
        std::pair<unsigned long, unsigned long> a (4, 9);
        ASSERT_EQ(a, allEdges[1]);
    }

    void graphWithXEntriesRandomSet(unsigned long x){
        DKTree dktree;
        vector<unsigned long> insertedEntries;
        for (unsigned long i = 0; i < x; i++) {
            dktree.insertEntry();
            insertedEntries.push_back(i);
        }
        std::cout <<  "graph made \n";
        vector<std::pair<unsigned long, unsigned long>> allEdges;
        for (unsigned long i = 0; i < x; i++) {
            for (unsigned long j = 0; j < x; j++) {
                int random = rand() % 100;
                if(random == 9){
                    dktree.addEdge(i, j);
                    allEdges.emplace_back(i, j);
                }
            }
        }
        unsigned long nrOfEdges = allEdges.size();
        std::cout <<  nrOfEdges << " edges added\n";
        vector<std::pair<unsigned long, unsigned long>> findings = dktree.reportAllEdges(insertedEntries, insertedEntries);
        std::cout <<  "edges reported  \n";
        sort(findings.begin(), findings.end());
        std::cout <<  "edges sorted \n";
        ASSERT_EQ(allEdges.size(), findings.size());
        std::cout <<  "correct size \n";
        for (unsigned long i = 0; i < nrOfEdges; i++) {
            ASSERT_EQ(allEdges[i], findings[i]);
        }
    }

    void graphWithXEntriesRandomDeleteAndFind(unsigned long x){
        DKTree dktree;
        vector<unsigned long> insertedEntries;
        for (unsigned long i = 0; i < x; i++) {
            dktree.insertEntry();
            insertedEntries.push_back(i);
        }
        std::cout <<  "graph made \n";
        vector<std::pair<unsigned long, unsigned long>> allEdges;
        for (unsigned long i = 0; i < x; i++) {
            for (unsigned long j = 0; j < x; j++) {
                int random = rand() % 100;
                if(random == 9){
                    dktree.addEdge(i, j);
                    allEdges.emplace_back(i, j);
                }
            }
        }
        unsigned long nrOfEdges = allEdges.size();
        std::cout <<  nrOfEdges << " edges added\n";
        vector<std::pair<unsigned long, unsigned long>> findings = dktree.reportAllEdges(insertedEntries, insertedEntries);
        std::cout <<  "edges reported  \n";
        sort(findings.begin(), findings.end());
        std::cout <<  "edges sorted \n";
        ASSERT_EQ(allEdges.size(), findings.size());
        std::cout <<  "correct size \n";
        for (unsigned long i = 0; i < nrOfEdges; i++) {
            ASSERT_EQ(allEdges[i], findings[i]);
        }

        unsigned long random = rand() % nrOfEdges;
        unsigned long entryToBeDeleted = allEdges[random].first;
        dktree.deleteEntry(entryToBeDeleted);
        insertedEntries.erase(insertedEntries.begin()+entryToBeDeleted);
        findings = dktree.reportAllEdges(insertedEntries, insertedEntries);
        sort(findings.begin(), findings.end());
        int i = 0;
        for(auto edge: allEdges){
            if(edge.first == entryToBeDeleted || edge.second == entryToBeDeleted){
                ASSERT_FALSE(edge == findings[i]);
            }else{
                ASSERT_EQ(edge, findings[i]);
                i++;
            }
        }
    }

    TEST(DKTreeTest, randomThousandGraph){
        std::cout << "randomThousandGraph test\n";
        graphWithXEntriesRandomSet(1000);
    }

    TEST(DKTreeTest, randomThousandGraphDeleteEntry){
        std::cout << "randomThousandGraphDeleteEntry test\n";
        graphWithXEntriesRandomDeleteAndFind(1000);
    }

    TEST(DKTreeTest, randomTenThousandGraph){
        std::cout << "randomTenThousandGraph test\n";
        graphWithXEntriesRandomSet(10000);
    }

    TEST(DKTreeTest, randomTenThousandGraphDeleteEntry){
        std::cout << "randomTenThousandGraphDeleteEntry test\n";
        graphWithXEntriesRandomDeleteAndFind(10000);
    }

    TEST(DKTreeTest, randomHundredThousandGraphDeleteEntry){
        std::cout << "randomHundredThousandGraphDeleteEntry test\n";
        graphWithXEntriesRandomDeleteAndFind(100000);
    }

}
#pragma clang diagnostic pop