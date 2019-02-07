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

    traverseToFirst0OrEndOfTTree(row, column, iteration, position, centry);

    // if the last bit found in the ttree is a 1 then find the final result in the ltree and set it to 1
    if (centry) {
        unsigned long ltreePosition = position - ttree->bits();
        ltree->setBit(ltreePosition, true);
    } else { // if not then change it to a 1 and insert new blocks where necessary
        ttree->setBit(position, true);
        //  std::cout << "set bit " << position << "\n";
        iteration++;
        //  std::cout << "iteration " << iteration << "\n";
        unsigned long blocksize = matrixSize / pow(k, iteration);
        //  std::cout << "blocksize " << blocksize << "\n";
        while (blocksize > 1) {
            // position +1 since paper has rank including the position, but function is exclusive position
            unsigned long insertAt = ttree->rank1(position + 1) * k * k;
            //       std::cout << "insert at " << insertAt << "\n";
            insertBlockTtree(insertAt);
            // printtt();
            unsigned long offset = calculateOffset(row, column, iteration);
            //       std::cout << "offset " << offset << "\n";
            position = insertAt + offset;
            //       std::cout << "position " << position << "\n";
            ttree->setBit(position, true);
            //        std::cout << "set bit " << position << "\n";
            iteration++;
            //      std::cout << "iteration " << iteration << "\n";
            blocksize = matrixSize / pow(k, iteration);
            //     std::cout << "blocksize " << blocksize << "\n";
        }
        // position +1 since paper has rank including the position, but function is exclusive position
        //  printtt();
        unsigned long lTreeinsertAt = (ttree->rank1(position + 1) * k * k) - ttree->bits();
        //  std::cout << "ltree insert at " << lTreeinsertAt << "\n";
        insertBlockLtree(lTreeinsertAt);
        //  printtt();
        unsigned long offset = calculateOffset(row, column, iteration);
        //  std::cout << "offset " << offset << "\n";
        position = lTreeinsertAt + offset;
        //  std::cout << "set bit " << position << "\n";
        ltree->setBit(position, true);
    }
}


void DKTree::removeEdge(unsigned long row, unsigned long column) {

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

}

bool DKTree::reportEdge(unsigned long a, unsigned long b) {
    //  std::cout << "report edge for a " << a << " b " << b << "\n";
    // test if both positions exist
    checkArgument(a, "reportEdge");
    checkArgument(b, "reportEdge");

    unsigned long iteration = 1;
    unsigned long tmax = ttree->bits();
    // the first position is 0+offset of the first iteration
    unsigned long position = calculateOffset(a, b, iteration);;
    bool centry = true; // to get the loop started

    traverseToFirst0OrEndOfTTree(a, b, iteration, position, centry);
    // std::cout << "endoftraverse, centry =  " << centry << "\n";
    // if the last bit found in the ttree is a 1 then find the final result in the ltree
    if (centry) {
        unsigned long ltreePosition = position - tmax;
        centry = ltree->access(ltreePosition);
    }
    return centry;
}


void DKTree::printtt() {
    cout << "ttree: ";
    printtree(ttree);
    cout << "ltree:";
    printtree(ltree);
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
        printf("\n");
    } else {
        for (auto &entry : tree->node.internalNode->entries) {
            if (entry.P == nullptr) {
                break;
            }
            printf("%s| -- (%lu, %lu)\n", prefix.c_str(), entry.b, entry.o);
            printtree(entry.P, depth + 1);
        }
    }
}


void DKTree::increaseMatrixSize() {
    const unsigned long FIRSTBIT = 0;
    // if the matrix is full, increase the size by multiplying with k
    matrixSize *= k;
    // position +1 since paper has rank including the position, but function is exclusive position
    if (ttree->findLeaf(FIRSTBIT).P->node.leafNode->bv.rank1((k * k) + 1) > 0) {
        // if there already is a 1 somewhere in the matrix, add a new block
        // in front of the bitvector and set the first bit to 1
        insertBlockTtree(FIRSTBIT);
        ttree->setBit(FIRSTBIT, true);
    }
}

unsigned long
DKTree::calculateOffset(const unsigned long row, const unsigned long column, const unsigned long iteration) {
    // first remove the rows and columns not beloning to the current block
    //  std::cout << "calculate offset for row " << row << " column " << column << " iter"  << iteration<< "\n";
    unsigned long formerPartitionSize = matrixSize / pow(k, iteration - 1);
    //  std::cout << "fps " << formerPartitionSize << "\n";
    unsigned long rowInBlock = row % formerPartitionSize;
    //  std::cout << "rib " << rowInBlock << "\n";
    unsigned long columnInBlock = column % formerPartitionSize;
    //   std::cout << "cib " << columnInBlock << "\n";
    //calculate the offset, each row partition adds k to the offset, each column partition 1
    unsigned long partitionSize = matrixSize / pow(k, iteration);
//    std::cout << "ps " << partitionSize << "\n";
    if (partitionSize == 0) {
        throw std::invalid_argument("partition size is 0\n");
    }
    unsigned long rowOffset = k * (rowInBlock / partitionSize);
    //  std::cout << "roff " << rowOffset << "\n";
    unsigned long columnOffset = columnInBlock / partitionSize;
    //  std::cout << "coff " << columnOffset << "\n";
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
        //std::cout << "position =  " << position << " centry = "<<centry<<"\n";
        if (centry) {
            iteration++;
            unsigned long offset = calculateOffset(row, column, iteration);
            // position +1 since paper has rank including the position, but function is exclusive position
            unsigned long positionOfFirst = ttree->rank1(position + 1) * k * k;
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