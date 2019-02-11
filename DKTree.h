//
// Created by anneke on 05/02/19.
//

#ifndef DK2TREE_DKTREE_H
#define DK2TREE_DKTREE_H

#include <string>
#include <utility>
#include <bits/stdc++.h>

#include "TTree.h"

class VectorData {
public:
    const vector<unsigned long> &entry;
    int start;
    int end;
    unsigned long iteration;
    unsigned long firstAt;

    VectorData(vector<unsigned long> &aEntry, int aStart, int aEnd, unsigned long aIteration,
               unsigned long aFirstAt)
            : entry(aEntry), start(aStart), end(aEnd), iteration(aIteration), firstAt(aFirstAt) {}

    // constructor for a first iteration, it includes the whole vector and always starts at the first bit of the ttree
    explicit VectorData(vector<unsigned long> &aEntry)
            : entry(aEntry), start(0), end(aEntry.size()), iteration(1), firstAt(0) {}

    VectorData(VectorData &vectorData, int aStart, int aEnd, unsigned long aIteration, unsigned long aFirstAt)
            : entry(vectorData.entry), start(aStart), end(aEnd), iteration(aIteration), firstAt(aFirstAt) {}
};


class DKTree final {

private:
    TTree *ttree;
    TTree *ltree;
    std::vector<unsigned long> freeColumns;
    unsigned long firstFreeColumn;
    unsigned long matrixSize;
    static constexpr unsigned long k = 2;
    static constexpr unsigned long k2 = k * k;

public:

    DKTree();

    ~DKTree();


    /**
     * Adds an edge from a to b
     * @param a the row number of the edge to be added
     * @param b the column number of the edge to be added
     */
    void addEdge(unsigned long row, unsigned long column);


/**
     * Removes the edge from a to b
     * @param a the row number of the edge to be added
     * @param b the column number of the edge to be added
     */
    void removeEdge(unsigned long row, unsigned long column);

    /**
     *  Inserts a column/row at the first empty column/row in the maxtrix, or if none at the end.
     *  @return the position at which the column was inserted.
     */
    unsigned long insertEntry();


    /**
     * Deletes column/row a from the matrix
     * @param a the column/row number to be deleted from the matrix
     */
    void deleteEntry(unsigned long a);


    /**
     * Reports all edges between a element of A, and b element of B.
     * @param A
     * @param B
     * @return TOdo
     */
    vector<std::pair<unsigned long, unsigned long>>
    reportAllEdges(const vector<unsigned long> &A, const vector<unsigned long> &B);


    /**
     * Reports whether or not there is an edge between a and b.
     * @param a
     * @param b
     * @return true if there is an edge between a and b, false otherwise
     */
    bool reportEdge(unsigned long a, unsigned long b);

    void printtt();

    DKTree(const DKTree &) = delete; // not allowed to use the copy constructor

private:
    void printtree(TTree *tree, unsigned long depth = 0);

    unsigned long calculateOffset(unsigned long row, unsigned long column, unsigned long iteration);

    void increaseMatrixSize();

    void checkArgument(unsigned long a, std::string functionName);

    void insertBlockTtree(unsigned long position);

    void insertBlockLtree(unsigned long position);

    void deleteBlockTtree(unsigned long position);

    void deleteBlockLtree(unsigned long position);

    bool
    deleteThisEdge(unsigned long row, unsigned long column, unsigned long iteration, unsigned long positionOfFirst);

    void
    traverseToFirst0OrEndOfTTree(unsigned long row, unsigned long column, unsigned long &iteration,
                                 unsigned long &position,
                                 bool &centry);

    bool deleteLTreeEdge(unsigned long positionOfFirst, unsigned long offset);

    bool deleteTTreeEdge(unsigned long row, unsigned long column, unsigned long iteration,
                         unsigned long positionOfFirst, unsigned long offset);

    void findAllEdges(const vector<unsigned long> &rows, const vector<unsigned long> &columns,
                      unsigned long &firstAt, unsigned long &iteration,
                      vector<std::pair<unsigned long, unsigned long>> &findings);

    void findAllEdges(VectorData &rows, VectorData &columns,
                      vector<std::pair<unsigned long, unsigned long>> &findings);

    void sortAndCheckVector(vector<unsigned long> &element);


    void
    splitEntriesOnOffset(const VectorData &entries, unsigned long partitionSize, int *entrieStart,
                         int *entrieEnd) const;

    void findEdgesInLTree(const VectorData &rows, const VectorData &columns,
                          vector<std::pair<unsigned long, unsigned long>> &findings);

    bool deleteEdges( VectorData &rows,  VectorData &columns);
};

#endif //DK2TREE_DKTREE_H
