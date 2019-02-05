//
// Created by anneke on 05/02/19.
//

#ifndef DK2TREE_DKTREE_H
#define DK2TREE_DKTREE_H


#include "TTree.h"

class DKTree {

private:TTree *ttree = new TTree();
private: TTree *ltree = new TTree();
private:std::vector<unsigned long> freeColumns;
private: unsigned long FirstFreeColumn = 0;

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

};


#endif //DK2TREE_DKTREE_H
