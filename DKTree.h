//
// Created by anneke on 05/02/19.
//

#ifndef DK2TREE_DKTREE_H
#define DK2TREE_DKTREE_H

#include <string>

#include "TTree.h"


class DKTree final {

private:
    TTree *ttree;
    TTree *ltree;
    std::vector<unsigned long> freeColumns;
    unsigned long firstFreeColumn;
    unsigned long matrixSize;
    static constexpr unsigned long k =2;

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
     * Reports all edges between a, aFirst <= a <= aLast, and b, bFirst <= b <= bLast.
     * @param aFirst
     * @param aLast
     * @param bFirst
     * @param bLast
     * @return TOdo
     */
    void reportAllEdges(unsigned long aFirst, unsigned long aLast, unsigned long bFirst, unsigned long bLast);

    /**
     * Reports whether or not there is an edge between a and b.
     * @param a
     * @param b
     * @return true if there is an edge between a and b, false otherwise
     */
    bool reportEdge(unsigned long a, unsigned long b);

    void printtt();

    DKTree(const DKTree&) = delete; // not allowed to use the copy constructor

private:
    void printtree(TTree *tree, unsigned long depth=0);
    unsigned long calculateOffset(unsigned long row, unsigned long column, unsigned long iteration);
    void increaseMatrixSize();
    void checkArgument(unsigned long a, std::string functionName);
    void insertBlockTtree(unsigned long position);
    void insertBlockLtree(unsigned long position);

    void
    traverseToFirst0OrEndOfTTree(unsigned long row, unsigned long column, unsigned long &iteration, unsigned long &position,
                                 bool &centry);
};


#endif //DK2TREE_DKTREE_H
