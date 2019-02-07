//
// Created by anneke on 05/02/19.
//

#include "DKTree.h"
#include <iostream>
#include <cmath>
using namespace std;

DKTree::DKTree() : ttree(new TTree()),ltree(new TTree()), freeColumns(),firstFreeColumn(0), k(2), matrixSize(pow(k,4)){
    ttree->insertBlock(0);
}

DKTree::~DKTree() {
    delete ttree;
    delete ltree;
}

void DKTree::addEdge(unsigned long row, unsigned long column) {
//    unsigned long position = 0;
//    unsigned long ttreeSize = ttree->bits();
//    bool edge = 1;
//    while (edge == 1 && position <= ttreeSize){
//        edge = ttree->access(position);
//
//    }
}

void DKTree::removeEdge(unsigned long row, unsigned long column) {

}

unsigned long DKTree::insertEntry() {
    unsigned long insertedColumn;
    if(!freeColumns.empty()){
        // if a column in the middle was freed earlier then first use this column
        insertedColumn = freeColumns.front();
        freeColumns.erase(freeColumns.begin());
    }else{
        if (firstFreeColumn > matrixSize){
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
    // TODO iff a of b is larger than firstfreecolumn of is in the freecolums list report an error
    unsigned long iteration = 1;
    unsigned long tmax = ttree->bits();
    // the first position is 0+offset of the first iteration
    unsigned long position = calculateOffset(a, b, iteration);;
    bool centry = true; // to get the loop started

    // while the current position is a 1 and the end of the ttree is not reached, access the next bit
    while(centry && position <= tmax){
        centry = ttree->access(position);
        if(centry){
            iteration++;
            unsigned long offset = calculateOffset(a, b, iteration);
            unsigned long positionOfFirst = ttree->rank1(position)*k*k;
            position = positionOfFirst + offset;
        }
    }

    // if the last bit found in the ttree is a 1 then find the final result in the ltree
    if(centry){
        unsigned long ltreePosition = position - tmax;
        centry = ltree->access(ltreePosition);
    }
    return centry;
}


void DKTree::printtt() {
    cout <<"ttree: ";
    printtree(ttree);
    cout <<"ltree:";
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
    // if the matrix is full, increase the size by multiplying with k
    matrixSize *= k;
    if(ttree->findLeaf(0).P->node.leafNode->bv.rank1(k*k) > 0) {
        // if there already is a 1 somewhere in the matrix, add a new block
        // in front of the bitvector and set the first bit to 1
        TTree *newRoot = ttree->insertBlock(0);
        if(newRoot != nullptr){
            ttree = newRoot;
        }
        ttree->setBit(0, true);
    }
}

unsigned long DKTree::calculateOffset(unsigned long row, unsigned long column, unsigned long iteration) {
    // first remove the rows and columns not beloning to the current block
    unsigned long formerPartitionSize = matrixSize/pow(k, iteration-1);
    unsigned long rowInBlock = row % formerPartitionSize;
    unsigned long columnInBlock = column % formerPartitionSize;
    // calculate the offset, each row partition adds k to the offset, each column partition 1
    unsigned long partitionSize = matrixSize/pow(k, iteration);
    unsigned long rowOffset = k * rowInBlock / partitionSize;
    unsigned long columnOffset = columnInBlock / partitionSize;
    return rowOffset+columnOffset;
}