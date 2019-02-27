//
// Created by anneke on 05/02/19.
//

#include <iostream>
#include <sstream>

#include "DKTree.h"

using namespace std;

unsigned long long_pow(unsigned long a, unsigned long b) {
    unsigned long result = 1;
    while (b != 0) {
        if (b % 2 != 0) {
            result *= a;
        }
        b >>= 1;
        a *= a;
    }
    return result;
}

DKTree::DKTree() : ttree(new TTree()), ltree(new LTree()), freeColumns(), firstFreeColumn(0), matrixSize(long_pow(k, 4ul)) {
    ttree->insertBlock(0);
}

DKTree::DKTree(unsigned long power) : ttree(new TTree()), ltree(new LTree()), freeColumns(), firstFreeColumn(0),
                                      matrixSize(long_pow(k, power)) {
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
        ltree->setBit(ltreePosition, true, &lPath);
    } else { // if not then change it to a 1 and insert new blocks where necessary
        ttree->setBit(position, true, &tPath);
        iteration++;
        unsigned long blocksize = matrixSize / long_pow(k, iteration);
        while (blocksize > 1) {
            // position +1 since paper has rank including the position, but function is exclusive position
            unsigned long insertAt = ttree->rank1(position + 1, &tPath) * BLOCK_SIZE;
            insertBlockTtree(insertAt);
            unsigned long offset = calculateOffset(row, column, iteration);
            position = insertAt + offset;
            ttree->setBit(position, true, &tPath);
            iteration++;
            blocksize = matrixSize / long_pow(k, iteration);
        }
        // position +1 since paper has rank including the position, but function is exclusive position
        unsigned long lTreeinsertAt = (ttree->rank1(position + 1, &tPath) * BLOCK_SIZE) - ttree->bits();
        insertBlockLtree(lTreeinsertAt);
        unsigned long offset = calculateOffset(row, column, iteration);
        position = lTreeinsertAt + offset;
        ltree->setBit(position, true, &lPath);
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
    } else if (ttree->access(positionOfFirst + offset, &tPath)) {
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
    unsigned long nextPositionOfFirst = (ttree->rank1(positionOfFirst + offset + 1, &tPath)) * BLOCK_SIZE;
    bool newCurrentBit = deleteThisEdge(row, column, iteration + 1, nextPositionOfFirst);
    // if any of its children are still true this one will stay true and therefore so should its parent.
    if (newCurrentBit) {
        return true;
    }
    ttree->setBit(positionOfFirst + offset, false, &tPath);
    if (iteration > 1) {
        // if we arent in the first iteration, see if any of the nodes in this block is still true
        bool only0s = true;
        for (unsigned long i = 0; i < BLOCK_SIZE && only0s; i++) {
            if (ttree->access(positionOfFirst + i, &tPath)) {
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
    ltree->setBit(lTreePosition, false, &lPath);
    // check if there are any positive bits in this block
    bool only0s = true;
    for (unsigned long i = 0; i < BLOCK_SIZE && only0s; i++) {
        if (ltree->access(lTreePositionOfFirst + i, &lPath)) {
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
    checkArgument(a, "deleteEntry");
    vector<unsigned long> allOthers;
    vector<unsigned long> thisOne{a};
    for (unsigned long i = 0; i < firstFreeColumn; i++) {
        allOthers.push_back(i);
    }
    for (int i = freeColumns.size() - 1; i >= 0; i--) {
        allOthers.erase(allOthers.begin() + freeColumns[i]);
    }

    VectorData thisA(thisOne);
    VectorData others(allOthers);
    deleteEdges(thisA, others);
    deleteEdges(others, thisA);
    if (a == firstFreeColumn - 1) {
        firstFreeColumn--;
    } else {
        freeColumns.push_back(a);
        sort(freeColumns.begin(), freeColumns.end());
    }
}

bool DKTree::deleteEdges(VectorData &rows, VectorData &columns) {
    if (rows.firstAt != columns.firstAt || rows.iteration != columns.iteration) {
        std::stringstream error;
        error << "deleteEdges: rows and columns asynch\n";
        throw std::invalid_argument(error.str());
    }
    const unsigned long partitionSize = matrixSize / long_pow(k, rows.iteration);
    bool only0s = true;
    if (partitionSize > 1) { // we are looking at ttree stuff
        // sort the rows and columns according to which offsets they belong
        int rowStart[k];
        int rowEnd[k];
        int columnStart[k];
        int columnEnd[k];

        for (int i = 0; i < k; i++) {
            rowStart[i] = -1;
            rowEnd[i] = -1;
            columnStart[i] = -1;
            columnEnd[i] = -1;
        }

        splitEntriesOnOffset(rows, partitionSize, rowStart, rowEnd);
        splitEntriesOnOffset(columns, partitionSize, columnStart, columnEnd);

        // for each offset, there can be a relation if there is atleast one row and one column and if its value is not 0.
        for (int offset = BLOCK_SIZE - 1; offset >= 0; offset--) {
            unsigned long rowOffset = offset / k;
            // std::cout << "rowOffset " << rowOffset << "\n";
            unsigned long columnOffset = offset % k;
            // std::cout << "columnOffset " << columnOffset << "\n";
            unsigned long currentNode = rows.firstAt + offset;
            bool nodeSubtreeHasEdges = ttree->access(currentNode, &tPath);
            if (nodeSubtreeHasEdges) {
                if (!(rowStart[rowOffset] == -1 || columnStart[columnOffset] == -1)) {
                    // there can only be a relation if there is atleast 1 element in both of them
                    // rank function is exclusive so +1
                    unsigned long nextNode = ttree->rank1(currentNode + 1, &tPath) * BLOCK_SIZE;
                    // if there are edges in this subtree find the edges stored in the child nodes
                    unsigned long nextIteration = rows.iteration + 1;
                    VectorData rowData(rows, rowStart[rowOffset], rowEnd[rowOffset], nextIteration, nextNode);
                    VectorData columnData(columns, columnStart[columnOffset], columnEnd[columnOffset], nextIteration,
                                          nextNode);
                    // check if there are still edges left in its child nodes after deleting the edges from the rowData columnData
                    bool stillHasEdges = deleteEdges(rowData, columnData);
                    // if there still are edges in its child nodes then this node stays 1 and so should its parent
                    if (stillHasEdges) {
                        only0s = false;
                    } else {
                        // if there are no edges in its child nodes this edge can be set to 0
                        ttree->setBit(currentNode, false, &tPath);
                    }
                } else {
                    // if this offset is 1 and there is no edge to delete, its parent also should know there are still edges
                    only0s = false;
                }
            }
        }
        if (only0s && rows.iteration > 1) {
            // if there are no more edges in this block it can be deleted
            deleteBlockTtree(rows.firstAt);
        }

    } else { // we look at ltree stuff
        only0s = deleteEdgesFromLTree(rows, columns);
    }

    return !only0s;
}

bool DKTree::deleteEdgesFromLTree(VectorData &rows, VectorData &columns) {
    bool only0s = true;
    const unsigned long partitionSize = matrixSize / long_pow(k, rows.iteration);
    if (partitionSize > 1) {
        std::stringstream error;
        error << "findEdgesInLTree: not lTree iteration\n";
        throw std::invalid_argument(error.str());
    }
    unsigned long ltreeposition = rows.firstAt - ttree->bits();
    for (unsigned long i = rows.start; i < rows.end; i++) {
        for (unsigned long j = columns.start; j < columns.end; j++) {
            unsigned long offset = calculateOffset(rows.entry[i], columns.entry[j], rows.iteration);
            unsigned long nodePosition = ltreeposition + offset;
            ltree->setBit(nodePosition, false, &lPath);
        }
    }
    for (unsigned long offst = 0; offst < BLOCK_SIZE && only0s; offst++) {
        if (ltree->access(ltreeposition + offst, &lPath)) {
            only0s = false;
        }
    }
    if (only0s) {
        deleteBlockLtree(ltreeposition);
    }
    return only0s;
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
        centry = ltree->access(ltreePosition, &lPath);
    }
    return centry;
}

vector<std::pair<unsigned long, unsigned long>> DKTree::reportAllEdges(const vector<unsigned long> &A,
                                                                       const vector<unsigned long> &B) {
    vector<unsigned long> rowsA(A);
    vector<unsigned long> columnsB(B);
    sortAndCheckVector(rowsA);
    sortAndCheckVector(columnsB);

    VectorData rows(rowsA);
    VectorData columns(columnsB);
    vector<std::pair<unsigned long, unsigned long>> findings;
    findAllEdges(rows, columns, findings);
    return findings;
}


void
DKTree::findAllEdges(VectorData &rows, VectorData &columns,
                     vector<std::pair<unsigned long, unsigned long>> &findings) {
    if (rows.firstAt != columns.firstAt || rows.iteration != columns.iteration) {
        std::stringstream error;
        error << "findAllEdges: rows and columns asynch\n";
        throw std::invalid_argument(error.str());
    }
    const unsigned long partitionSize = matrixSize / long_pow(k, rows.iteration);
    if (partitionSize > 1) { // we are looking at ttree stuff
        // sort the rows and columns according to which offsets they belong
        int rowStart[k];
        int rowEnd[k];
        int columnStart[k];
        int columnEnd[k];

        for (int i = 0; i < k; i++) {
            rowStart[i] = -1;
            rowEnd[i] = -1;
            columnStart[i] = -1;
            columnEnd[i] = -1;
        }

        splitEntriesOnOffset(rows, partitionSize, rowStart, rowEnd);
        splitEntriesOnOffset(columns, partitionSize, columnStart, columnEnd);

        // for each offset, there can be a relation if there is atleast one row and one column and if its value is not 0.
        for (unsigned long offset = 0; offset < BLOCK_SIZE; offset++) {
            unsigned long rowOffset = offset / k;
            unsigned long columnOffset = offset % k;
            if (!(rowStart[rowOffset] == -1 || columnStart[columnOffset] == -1)) {
                // there can only be a relation if there is atleast 1 element in both of them
                unsigned long currentNode = rows.firstAt + offset;
                bool nodeSubtreeHasEdges = ttree->access(currentNode, &tPath);
                if (nodeSubtreeHasEdges) {
                    // rank function is exclusive so +1
                    unsigned long nextNode = ttree->rank1(currentNode + 1, &tPath) * BLOCK_SIZE;
                    // if there are edges in this subtree find the edges stored in the child nodes
                    unsigned long nextIteration = rows.iteration + 1;
                    VectorData rowData(rows, rowStart[rowOffset], rowEnd[rowOffset], nextIteration, nextNode);
                    VectorData columnData(columns, columnStart[columnOffset], columnEnd[columnOffset], nextIteration,
                                          nextNode);
                    findAllEdges(rowData, columnData, findings);
                }
            }
        }
    } else { // we look at ltree stuff
        findEdgesInLTree(rows, columns, findings);
    }
}

void DKTree::findEdgesInLTree(const VectorData &rows, const VectorData &columns,
                              vector<pair<unsigned long, unsigned long>> &findings) {

    const unsigned long partitionSize = matrixSize / long_pow(k, rows.iteration);
    if (partitionSize > 1) {
        std::stringstream error;
        error << "findEdgesInLTree: not lTree iteration\n";
        throw std::invalid_argument(error.str());
    }
    unsigned long ltreeposition = rows.firstAt - ttree->bits();
    for (unsigned long i = rows.start; i < rows.end; i++) {
        for (unsigned long j = columns.start; j < columns.end; j++) {
            unsigned long offset = calculateOffset(rows.entry[i], columns.entry[j], rows.iteration);
            unsigned long nodePosition = ltreeposition + offset;
            bool hasEdge = ltree->access(nodePosition, &lPath);
            if (hasEdge) {
                pair<unsigned long, unsigned long> edge(rows.entry[i], columns.entry[j]);
                findings.push_back(edge);
            }
        }
    }
}

void DKTree::splitEntriesOnOffset(const VectorData &entries, const unsigned long partitionSize, int *entrieStart,
                                  int *entrieEnd) const {
    const unsigned long formerPartitionSize = partitionSize * k;
    unsigned long offsetStarted;
    for (unsigned long i = entries.start; i < entries.end; i++) {
        unsigned long entrieInBlock = entries.entry[i] % formerPartitionSize;
        unsigned long entrieOffset = (entrieInBlock / partitionSize);
        if (entrieStart[entrieOffset] == -1) {
            entrieStart[entrieOffset] = i;
            if (i > entries.start) {
                entrieEnd[offsetStarted] = i;
            }
            offsetStarted = entrieOffset;
        }
        if (i == entries.end - 1) {
            entrieEnd[offsetStarted] = i + 1;
        }
    }
}


void DKTree::printtt() {
    cout << "ttree:" << endl;
    printttree(ttree);
    printf("\n");
    cout << "ltree:" << endl;

    printltree(ltree);
    printf("\n");
}

void DKTree::printttree(TTree *tree, unsigned long depth) {
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
        printf("%s (%lu bits, %lu ones)\n", prefix.c_str(), tree->node.internalNode->bits(), tree->node.internalNode->ones());
        for (auto &entry : tree->node.internalNode->entries) {
            if (entry.P == nullptr) {
                break;
            }
            printttree(entry.P, depth + 1);
        }
    }
}

void DKTree::printltree(LTree *tree, unsigned long depth) {
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
        printf("%s (%lu bits)\n", prefix.c_str(), tree->node.internalNode->bits());
        for (auto &entry : tree->node.internalNode->entries) {
            if (entry.P == nullptr) {
                break;
            }
            printltree(entry.P, depth + 1);
        }
    }
}

void DKTree::increaseMatrixSize() {
    const unsigned long FIRSTBIT = 0;
    // if the matrix is full, increase the size by multiplying with k
    matrixSize *= k;
    // position +1 since paper has rank including the position, but function is exclusive position
    if (ttree->rank1(BLOCK_SIZE, &tPath) > 0) {
        // if there already is a 1 somewhere in the matrix, add a new block
        // in front of the bitvector and set the first bit to 1
        insertBlockTtree(FIRSTBIT);
        ttree->setBit(FIRSTBIT, true, &tPath);
    }
}

unsigned long
DKTree::calculateOffset(const unsigned long row, const unsigned long column, const unsigned long iteration) {
    // first remove the rows and columns not beloning to the current block
    unsigned long formerPartitionSize = matrixSize / long_pow(k, iteration - 1);
    unsigned long rowInBlock = row % formerPartitionSize;
    unsigned long columnInBlock = column % formerPartitionSize;
    //calculate the offset, each row partition adds k to the offset, each column partition 1
    unsigned long partitionSize = matrixSize / long_pow(k, iteration);
    if (partitionSize == 0) {
        throw std::invalid_argument("partition size is 0\n");
    }
    unsigned long rowOffset = k * (rowInBlock / partitionSize);
    unsigned long columnOffset = columnInBlock / partitionSize;
    return rowOffset + columnOffset;
}

void DKTree::checkArgument(unsigned long a, std::string functionName) {
    if (a >= firstFreeColumn) {
        std::stringstream error;
        error << functionName << ": invalid argument " << a << ", position not occupied in matrix\n";
        throw std::invalid_argument(error.str());
    } else if (a < 0) {
        std::stringstream error;
        error << functionName << ": invalid argument " << a << ", position does not exist\n";
        throw std::invalid_argument(error.str());
    } else {
        for (auto &fc: freeColumns) {
            if (fc == a) {
                std::stringstream error;
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
        centry = ttree->access(position, &tPath);
        if (centry) {
            iteration++;
            unsigned long offset = calculateOffset(row, column, iteration);
            // position +1 since paper has rank including the position, but function is exclusive position
            unsigned long positionOfFirst = ttree->rank1(position + 1, &tPath) * BLOCK_SIZE;
            position = positionOfFirst + offset;
        }
    }
}

void DKTree::sortAndCheckVector(vector<unsigned long> &elements) {
    if (elements.empty()) {
        std::stringstream error;
        error << "sortAndCheckVector: invalid argument, empty input vector \n";
        throw std::invalid_argument(error.str());
    }
    // sort and delete doubles
    sort(elements.begin(), elements.end());
    elements.erase(unique(elements.begin(), elements.end()), elements.end());
    std::string functionname = "reportAllEdges";
    for (auto elemt:elements) {
        checkArgument(elemt, functionname);
    }
}

void DKTree::insertBlockTtree(unsigned long position) {
    TTree *newRoot = ttree->insertBlock(position, &tPath);
    if (newRoot != nullptr) {
        ttree = newRoot;
    }
    tPath.clear();
}

void DKTree::insertBlockLtree(unsigned long position) {
    LTree *newRoot = ltree->insertBlock(position, &lPath);
    if (newRoot != nullptr) {
        ltree = newRoot;
    }
    lPath.clear();
}

void DKTree::deleteBlockTtree(unsigned long position) {
    TTree *newRoot = ttree->deleteBlock(position, &tPath);
    if (newRoot != nullptr) {
        ttree = newRoot;
    }
    tPath.clear();
}

void DKTree::deleteBlockLtree(unsigned long position) {
    LTree *newRoot = ltree->deleteBlock(position, &lPath);
    if (newRoot != nullptr) {
        ltree = newRoot;
    }
    lPath.clear();
}

void measure(TTree* tree, unsigned long *bv, unsigned long *inodes, unsigned long *lnodes) {
    if (tree->isLeaf) {
        (*lnodes) ++;
        (*bv) += tree->node.leafNode->bits();
    } else {
        (*inodes)++;
        for (auto &entry : tree->node.internalNode->entries) {
            auto p = entry.P;
            if (p != nullptr) {
                measure(p, bv, inodes, lnodes);
            }
        }
    }
}

unsigned long DKTree::memoryUsage() {
    unsigned long bv = 0, inodes =0, lnodes = 0;
    measure(this->ttree, &bv, &inodes, &lnodes);
    printf("BitVector: %lu bits\n", bv);
    printf("%lu internal, %lu leaf nodes\n", inodes, lnodes);

    unsigned long m;
    unsigned long result = sizeof(DKTree);
    printf("DKTree: %lu\n", sizeof(DKTree));

    m = ttree->memoryUsage();
    printf("TTree: %lu\n", m);
    result += m;
    printf("  Of which bitvector: %lu\n", (ttree->bits() + 7) / 8);

    m = ltree->memoryUsage();
    printf("LTree: %lu\n", m);
    result += m;
    printf("  Of which bitvector: %lu\n", (ltree->bits() + 7) / 8);

    result += tPath.size() * sizeof(Nesbo);
    printf("TPath: %lu\n", tPath.size() * sizeof(Nesbo));
    result += lPath.size() * sizeof(LNesbo);
    printf("LPath: %lu\n", lPath.size() * sizeof(LNesbo));
    result += freeColumns.size() * sizeof(unsigned long);
    printf("Free Columns: %lu\n", freeColumns.size() * sizeof(unsigned long));
    return result;
}
