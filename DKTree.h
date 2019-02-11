//
// Created by anneke on 05/02/19.
//

#ifndef DK2TREE_DKTREE_H
#define DK2TREE_DKTREE_H

#include <string>
#include <utility>
#include <bits/stdc++.h>

#include "TTree.h"

// a class that contains a vector of entries in the matrix
// with a start and end such that the entries at index start <= i < end are all in the same block at when
// dividing the matrix size by k^(iteration-1), and the bit with offset 0 for this iteration is located at firstAt
class VectorData {
public:
    const vector<unsigned long> &entry;
    int start;
    int end;
    unsigned long iteration;
    unsigned long firstAt;

    // constructor for a first iteration, it includes the whole vector and always starts at the first bit of the ttree
    explicit VectorData(vector<unsigned long> &aEntry)
            : entry(aEntry), start(0), end(aEntry.size()), iteration(1), firstAt(0) {}

    VectorData(VectorData &vectorData, int aStart, int aEnd, unsigned long aIteration, unsigned long aFirstAt)
            : entry(vectorData.entry), start(aStart), end(aEnd), iteration(aIteration), firstAt(aFirstAt) {}
};


class DKTree final {

private:

    TTree *ttree; // the tree whose leaves contain the internal nodes of the k2 tree
    TTree *ltree; // the tree whose leaves contain the leave nodes of the k2 tree
    std::vector<unsigned long> freeColumns; // contains the entries in the matrix  below firstFreeColumn that are not in use
    unsigned long firstFreeColumn; // the lowest index above the used entries
    unsigned long matrixSize; // current size of the matrix, is a power of k
    static constexpr unsigned long k = 2;
    static constexpr unsigned long k2 = k * k; //k square

public:

    // initialises a tree for a matrix of size k^4
    DKTree();

    // initialises a dktree for a matrix of size k^power
    explicit DKTree(unsigned long power);

    ~DKTree();


    /**
     * Adds an edge from a to b
     * @param a the row of the edge to be added
     * @param b the column of the edge to be added
     * @throws illegal argument exception if row or column is not present in the matrix
     */
    void addEdge(unsigned long row, unsigned long column);


/**
     * Removes the edge from a to b
     * @param a the row of the edge to be added
     * @param b the column of the edge to be added
     * @throws illegal argument exception if row or column is not present in the matrix
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
     * @throws illegal argument exception if a is not present in the matrix
     */
    void deleteEntry(unsigned long a);


    /**
     * Reports all edges between a element of A, and b element of B.
     * @param A non empty, contains the first element of the pairs to be reported
     * @param B non empty, ontains the second element of the pairs to be reported
     * @return All pairs <a,b> such that a is an element of A and b is an element of B
     * @throws illegal argument exception if A or B is empty or if any of the elements in A or B is not present in the matrix
     */
    vector<std::pair<unsigned long, unsigned long>>
    reportAllEdges(const vector<unsigned long> &A, const vector<unsigned long> &B);


    /**
     * Reports whether or not there is an edge between a and b.
     * @param a first element of the edge to be reported
     * @param b second element of the edge to be reported
     * @return true if there is an edge from a to b, false otherwise
     */
    bool reportEdge(unsigned long a, unsigned long b);

    /**
    * prints the leaf nodes of the ttree and the ltree
    */
    void printtt();

    DKTree(const DKTree &) = delete; // not allowed to use the copy constructor

private:
    /**
    * prints the leaf nodes of the input tree
    * @param tree the tree to be printed
    */
    void printtree(TTree *tree, unsigned long depth = 0);

    /**
    * Calculates the offset of a row and column in a block at iteration iteration
    * @param row the row in the matrix
    * @param column the column in the matrix
    * @param iteration the iteration for which the offset needs to be calculated
    * @return the offset of the combination of row and column in a block at iteration iteration
    */
    unsigned long calculateOffset(unsigned long row, unsigned long column, unsigned long iteration);

    /**
     * Increase the size of the matrix, new matrixSize = oldMatrixSize * k
     */
    void increaseMatrixSize();

    /**
    * checks if a is present in the matrix, if not throws an exception
    * @param a the entry to be checked
    * @throws illegal argument exception if a is not present in the matrix
    */
    void checkArgument(unsigned long a, std::string functionName);

    /**
     * Inserts a block 4 0's (0000) at position position in the ttree
     * @param position the place at which the block of 0's should be inserted
     */
    void insertBlockTtree(unsigned long position);

    /**
     * Inserts a block 4 0's (0000) at position position in the tree
     * @param position the place at which the block of 0's should be inserted
     */
    void insertBlockLtree(unsigned long position);

    /**
     * Deletes 4 bits starting at position position in the ttree
     * @param position the place from which 4 bits should be deleted
     */
    void deleteBlockTtree(unsigned long position);

    /**
     * Deletes 4 bits starting at position position in the ltree
     * @param position the place from which 4 bits should be deleted
     */
    void deleteBlockLtree(unsigned long position);

    /**
    * Deletes row and column from the k2 tree, and reports if the k bits following positionOfFirst have any 1's
    * @param row the row in the matrix of the edge to be deleted
    * @param column the column in the matrix of the edge to be deleted
    * @param iteration to calculate the offset
    * @param positionOfFirst location of the first bit of the block in iteration iteration in which the edge is present
    * @return true if there are any 1's in the k bits starting at positionOfFirst after deletion of the edge, false otherwise
    */
    bool
    deleteThisEdge(unsigned long row, unsigned long column, unsigned long iteration, unsigned long positionOfFirst);

    /**
    * Traverses the ttree along the path of the k2 in the ttree to the entry of row/column till a 0 is found or untill the end of the ttree is reached
    * @param row the row in the matrix of the entry to be searched
    * @param column the column in the matrix of the entry to be searched
    * @param iteration to calculate the offset
    * @param position location of the first bit of the block in iteration iteration in which the edge is present
    * @param centry will contain true afterwards if the end of the ttree was reached, or false when a 0 was found
    */
    void
    traverseToFirst0OrEndOfTTree(unsigned long row, unsigned long column, unsigned long &iteration,
                                 unsigned long &position,
                                 bool &centry);

    /**
   * Deletes the edge (positionOfFirst+offset) from the ltree
   * @param positionOfFirst bit before the offset
   * @param offset together with the position of the first points to the edge to be deleted
   * @return true if there are any 1's in the k bits starting at positionOfFirst after deletion of the edge, false otherwise
   */
    bool deleteLTreeEdge(unsigned long positionOfFirst, unsigned long offset);

    /**
  * Deletes the edge (positionOfFirst+offset) from the ttree
  * @param positionOfFirst bit before the offset
  * @param offset together with the position of the first points to the edge to be deleted
  * @return true if there are any 1's in the k bits starting at positionOfFirst after deletion of the edge, false otherwise
  */
    bool deleteTTreeEdge(unsigned long row, unsigned long column, unsigned long iteration,
                         unsigned long positionOfFirst, unsigned long offset);

    /**
   * Finds all edges from row element of rows to column element of columns and stores them in findings
   * @param rows the rows in the matrix of the edges to be found
   * @param columns the columns in the matrix of the edges to be found
   * @param findings to store the edges found and contains edges already found
   */
    void findAllEdges(VectorData &rows, VectorData &columns,
                      vector<std::pair<unsigned long, unsigned long>> &findings);

    /**
  * checks that all elements in element are present in the matrix, sorts them and deletes doubles
  * @param element to be checked and sorted
  * @throw illegal argument exception if any of the arguments is not present in the matrix
  */
    void sortAndCheckVector(vector<unsigned long> &element);

/**
   * For each offset calculate the first and last entrie in the vector belonging to that offset, or -1 if none
   * @param entries to be split on offset
   * @param partitionSize number of rows/columns beloning to each offset
   * @param entrieStart to be filled with the start index for each offset or -1 if none
   * @param entrieEnd to be filled with the end index for each offset or -1 if none
   */
    void
    splitEntriesOnOffset(const VectorData &entries, unsigned long partitionSize, int *entrieStart,
                         int *entrieEnd) const;

    /**
   * Finds all edges from row element of rows to column element of columns and stores them in findings
   * @param rows the rows in the matrix of the edges to be found
   * @param columns the columns in the matrix of the edges to be found
   * @param findings to store the edges found and contains edges already found
   */
    void findEdgesInLTree(const VectorData &rows, const VectorData &columns,
                          vector<std::pair<unsigned long, unsigned long>> &findings);

    /**
   * Deletes all edges from row in rows to column in columns
   * @param rows the rows in the matrix of the edges to be deleted
   * @param columns the columns in the matrix of the edges to be deleted
   * @return true if after deleting this block still has 1's, false otherwise
   */
    bool deleteEdges(VectorData &rows, VectorData &columns);

    /**
   * Deletes all edges from row in rows to column in columns from the ltree
   * @param rows the rows in the matrix of the edges to be deleted
   * @param columns the columns in the matrix of the edges to be deleted
   * @return true if after deleting this block still has 1's, false otherwise
   */
    bool deleteEdgesFromLTree(VectorData &rows, VectorData &columns);
};

#endif //DK2TREE_DKTREE_H
