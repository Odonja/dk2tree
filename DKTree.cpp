//
// Created by anneke on 05/02/19.
//

#include <iostream>
#include <cmath>
#include <sstream>

#include "DKTree.h"

using namespace std;

DKTree::DKTree() : ttree(new TTree()), ltree(new TTree()), freeColumns(), firstFreeColumn(0), matrixSize(pow(k, 4)) {
    ttree->insertBlock(0);
}

DKTree::~DKTree() {
    delete ttree;
    delete ltree;
}

void DKTree::addEdge(unsigned long row, unsigned long column) {
    // test if both positions exist
    checkArgument(row, "addEdge");
    checkArgument(column, "addEdge");

    unsigned long iteration = 1;
    // the first position is 0+offset of the first iteration
    unsigned long position = calculateOffset(row, column, iteration);;
    bool centry = true; // to get the loop started

    // will change the values of iteration, position, and centry
    traverseToFirst0OrEndOfTTree(row, column, iteration, position, centry);

    // if the last bit found in the ttree is a 1 then find the final result in the ltree and set it to 1
    if (centry) {
        unsigned long ltreePosition = position - ttree->bits();
        ltree->setBit(ltreePosition, true);
    } else { // if not then change it to a 1 and insert new blocks where necessary
        ttree->setBit(position, true);
        iteration++;
        unsigned long blocksize = matrixSize / pow(k, iteration);
        while (blocksize > 1) {
            // position +1 since paper has rank including the position, but function is exclusive position
            unsigned long insertAt = ttree->rank1(position + 1) * k2;
            insertBlockTtree(insertAt);
            unsigned long offset = calculateOffset(row, column, iteration);
            position = insertAt + offset;
            ttree->setBit(position, true);
            iteration++;
            blocksize = matrixSize / pow(k, iteration);
        }
        // position +1 since paper has rank including the position, but function is exclusive position
        unsigned long lTreeinsertAt = (ttree->rank1(position + 1) * k2) - ttree->bits();
        insertBlockLtree(lTreeinsertAt);
        unsigned long offset = calculateOffset(row, column, iteration);
        position = lTreeinsertAt + offset;
        ltree->setBit(position, true);
    }
}


void DKTree::removeEdge(unsigned long row, unsigned long column) {
    const unsigned long POSITIONOFFIRST = 0;
    const unsigned long FIRSTITERATION = 1;
    deleteThisEdge(row, column, FIRSTITERATION, POSITIONOFFIRST);
}

bool DKTree::deleteThisEdge(const unsigned long row, const unsigned long column, const unsigned long iteration,
                            const unsigned long positionOfFirst) {
    unsigned long offset = calculateOffset(row, column, iteration);
    if (positionOfFirst >= ttree->bits()) {
        return deleteLTreeEdge(positionOfFirst, offset);
    } else if (ttree->access(positionOfFirst + offset)) {
        return deleteTTreeEdge(row, column, iteration, positionOfFirst, offset);
    } else {
        // the current ttree bit is already false, so no changes should be made, as it came here it parent should
        // be true and therefore stay true
        return true;
    }

}

bool DKTree::deleteTTreeEdge(const unsigned long row, const unsigned long column, const unsigned long iteration,
                             const unsigned long positionOfFirst, unsigned long offset) {
    // if the current position is true then check if after deleting the next edge any of its children are still true
    unsigned long nextPositionOfFirst = (ttree->rank1(positionOfFirst + offset + 1)) * k2;
    bool newCurrentBit = deleteThisEdge(row, column, iteration + 1, nextPositionOfFirst);
    // if any of its children are still true this one will stay true and therefore so should its parent.
    if (newCurrentBit) {
        return true;
    }
    ttree->setBit(positionOfFirst + offset, false);
    if (iteration > 1) {
        // if we arent in the first iteration, see if any of the nodes in this block is still true
        bool only0s = true;
        for (unsigned long i = 0; i < k2 && only0s; i++) {
            if (ttree->access(positionOfFirst + i)) {
                only0s = false;
            }
        }
        // if all nodes are false this block can be deleted
        if (only0s) {
            deleteBlockTtree(positionOfFirst);
        }
        return !only0s; // if all bits in this block are false the parent should be false, else it should be true.
    }
    // we are in the first iteration, so no bits should be deleted
    return true;
}

bool DKTree::deleteLTreeEdge(const unsigned long positionOfFirst, unsigned long offset) {
    // if the position is in the ltree, set the bit to false in the ltree
    unsigned long lTreePositionOfFirst = positionOfFirst - ttree->bits();
    unsigned long lTreePosition = lTreePositionOfFirst + offset;
    ltree->setBit(lTreePosition, false);
    // check if there are any positive bits in this block
    bool only0s = true;
    for (unsigned long i = 0; i < k2 && only0s; i++) {
        if (ltree->access(lTreePositionOfFirst + i)) {
            only0s = false;
        }
    }
    // iff all bits in the block are 0 delete this block
    if (only0s) {
        deleteBlockLtree(lTreePositionOfFirst);
    }
    return !only0s; // if this block is all false then its parent should be false, else the parent should be true
}

unsigned long DKTree::insertEntry() {
    unsigned long insertedColumn;
    if (!freeColumns.empty()) {
        // if a column in the middle was freed earlier then first use this column
        insertedColumn = freeColumns.front();
        freeColumns.erase(freeColumns.begin());
    } else {
        if (firstFreeColumn > matrixSize) {
            increaseMatrixSize();
        }
        // if not use the last column
        insertedColumn = firstFreeColumn;
        firstFreeColumn++;
    }
    // as unused places in the matrix have 0 everywhere no action on the bitvector is needed
    return insertedColumn;
}


void DKTree::deleteEntry(unsigned long a) {
//TODO
}

bool DKTree::reportEdge(unsigned long a, unsigned long b) {
    // test if both positions exist
    checkArgument(a, "reportEdge");
    checkArgument(b, "reportEdge");

    unsigned long iteration = 1;
    unsigned long tmax = ttree->bits();
    // the first position is 0+offset of the first iteration
    unsigned long position = calculateOffset(a, b, iteration);;
    bool centry = true; // to get the loop started

    traverseToFirst0OrEndOfTTree(a, b, iteration, position, centry);
    // if the last bit found in the ttree is a 1 then find the final result in the ltree
    if (centry) {
        unsigned long ltreePosition = position - tmax;
        centry = ltree->access(ltreePosition);
    }
    return centry;
}

vector<std::pair<unsigned long, unsigned long>> DKTree::reportAllEdges(vector<unsigned long> &A,
                                                                       vector<unsigned long> &B) {
    //TODO add a check and sort and optimize
    unsigned long FIRSTBIT = 0;
    unsigned long FIRSTITERATION = 1;
    vector<std::pair<unsigned long, unsigned long>> findings;
    findAllEdges(A, B, FIRSTBIT, FIRSTITERATION, findings);
    return findings;
}

void
DKTree::findAllEdges(vector<unsigned long> &rows, vector<unsigned long> &columns,
                     unsigned long &firstAt, unsigned long &iteration,
                     vector<std::pair<unsigned long, unsigned long>> &findings) {
    const unsigned long partitionSize = matrixSize / pow(k, iteration);
    if (partitionSize > 1) { // we are looking at ttree stuff
        // sort the rows and columns according to which offsets they belong
        vector<unsigned long> rowsForOffset[k2];
        vector<unsigned long> columnsForOffset[k2];
        const unsigned long formerPartitionSize = partitionSize * k;

        for (auto row:rows) {
            unsigned long rowInBlock = row % formerPartitionSize;
            unsigned long rowOffset = (rowInBlock / partitionSize) * k;
            for (unsigned long i = 0; i < k; i++) {
                rowsForOffset[rowOffset + i].push_back(row);
            }
        }

        for (auto column:columns) {
            unsigned long columnInBlock = column % formerPartitionSize;
            unsigned long columnOffset = (columnInBlock / partitionSize);
            for (unsigned long i = 0; i < k; i++) {
                columnsForOffset[columnOffset + i*k].push_back(column);
            }
        }

//        for (unsigned long offset = 0; offset < k2; offset++) {
//            std::cout << "offset " << offset <<":\n rows: ";
//            for (auto row:rowsForOffset[offset]) {
//                std::cout << row << " ";
//            }
//            std::cout <<"\n columns: ";
//            for (auto column:columnsForOffset[offset]) {
//                std::cout << column << " ";
//            }
//            std::cout <<"\n" ;
//        }

        // for each offset, there can be a relation if there is atleast one row and one column and if its value is not 0.
        for (unsigned long offset = 0; offset < k2; offset++) {
            if (!(rowsForOffset[offset].empty() || columnsForOffset[offset].empty())) {
                // there can only be a relation if there is atleast 1 element in both of them
                unsigned long currentNode = firstAt + offset;
                bool nodeSubtreeHasEdges = ttree->access(currentNode);
                if (nodeSubtreeHasEdges) {
                    // rank function is exclusive so +1
                    unsigned long nextNode = ttree->rank1(currentNode + 1) * k2;
                    // if there are edges in this subtree find the edges stored in the child nodes
                    unsigned long nextIteration = iteration + 1;
                    findAllEdges(rowsForOffset[offset], columnsForOffset[offset], nextNode, nextIteration,
                                 findings);
                }
            }
        }
    } else { // we look at ltree stuff
        unsigned long ltreeposition = firstAt - ttree->bits();
        for (auto row : rows) {
            for (auto column:columns) {
                unsigned long offset = calculateOffset(row, column, iteration);
                unsigned long nodePosition = ltreeposition + offset;
                bool hasEdge = ltree->access(nodePosition);
                if (hasEdge) {
                    std::pair<unsigned long, unsigned long> edge(row, column);
                    findings.push_back(edge);
                }
            }
        }
    }
}


void DKTree::printtt() {
    cout << "ttree: ";
    printtree(ttree);
    printf("\n");
    cout << "ltree:";
    printtree(ltree);
    printf("\n");
}

void DKTree::printtree(TTree *tree, unsigned long depth) {
    std::string prefix;
    for (unsigned long i = 0; i < depth; i++) {
        prefix += "| ";
    }
    if (tree->isLeaf) {
        auto &bv = tree->node.leafNode->bv;
        printf("%s", prefix.c_str());
        for (auto b : bv.data) {
            printf("%i", (bool) b);
        }

    } else {
        for (auto &entry : tree->node.internalNode->entries) {
            if (entry.P == nullptr) {
                break;
            }
            printtree(entry.P, depth + 1);
        }
    }
}


void DKTree::increaseMatrixSize() {
    const unsigned long FIRSTBIT = 0;
    // if the matrix is full, increase the size by multiplying with k
    matrixSize *= k;
    // position +1 since paper has rank including the position, but function is exclusive position
    if (ttree->rank1(k2) > 0) {
        // if there already is a 1 somewhere in the matrix, add a new block
        // in front of the bitvector and set the first bit to 1
        insertBlockTtree(FIRSTBIT);
        ttree->setBit(FIRSTBIT, true);
    }
}

unsigned long
DKTree::calculateOffset(const unsigned long row, const unsigned long column, const unsigned long iteration) {
    // first remove the rows and columns not beloning to the current block
    unsigned long formerPartitionSize = matrixSize / pow(k, iteration - 1);
    unsigned long rowInBlock = row % formerPartitionSize;
    unsigned long columnInBlock = column % formerPartitionSize;
    //calculate the offset, each row partition adds k to the offset, each column partition 1
    unsigned long partitionSize = matrixSize / pow(k, iteration);
    if (partitionSize == 0) {
        throw std::invalid_argument("partition size is 0\n");
    }
    unsigned long rowOffset = k * (rowInBlock / partitionSize);
    unsigned long columnOffset = columnInBlock / partitionSize;
    return rowOffset + columnOffset;
}

void DKTree::checkArgument(unsigned long a, std::string functionName) {
    std::stringstream error;
    if (a >= firstFreeColumn) {
        error << functionName << ": invalid argument " << a << ", position not occupied in matrix\n";
        throw std::invalid_argument(error.str());
    } else {
        for (auto &fc: freeColumns) {
            if (fc == a) {
                error << functionName << ": invalid argument " << a << ", position was deleted from matrix\n";
                throw std::invalid_argument(error.str());
            }
        }
    }
}

void DKTree::traverseToFirst0OrEndOfTTree(unsigned long row, unsigned long column, unsigned long &iteration,
                                          unsigned long &position, bool &centry) {
    unsigned long tmax = ttree->bits();
    // while the current position is a 1 and the end of the ttree is not reached, access the next bit
    while (centry && position < tmax) {
        centry = ttree->access(position);
        if (centry) {
            iteration++;
            unsigned long offset = calculateOffset(row, column, iteration);
            // position +1 since paper has rank including the position, but function is exclusive position
            unsigned long positionOfFirst = ttree->rank1(position + 1) * k2;
            position = positionOfFirst + offset;
        }
    }
}

void DKTree::insertBlockTtree(unsigned long position) {
    TTree *newRoot = ttree->insertBlock(position);
    if (newRoot != nullptr) {
        ttree = newRoot;
    }
}

void DKTree::insertBlockLtree(unsigned long position) {
    TTree *newRoot = ltree->insertBlock(position);
    if (newRoot != nullptr) {
        ltree = newRoot;
    }
}

void DKTree::deleteBlockTtree(unsigned long position) {
    TTree *newRoot = ttree->deleteBlock(position);
    if (newRoot != nullptr) {
        ttree = newRoot;
    }
}

void DKTree::deleteBlockLtree(unsigned long position) {
    TTree *newRoot = ltree->deleteBlock(position);
    if (newRoot != nullptr) {
        ltree = newRoot;
    }
}
