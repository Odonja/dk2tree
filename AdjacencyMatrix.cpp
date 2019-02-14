//
// Created by hugo on 14-2-19.
//

#include "AdjacencyMatrix.h"

#include <algorithm>
#include <sstream>
#include "BinarySearch.h"

AdjacencyMatrix::AdjacencyMatrix() :
    size(0),
    matrix(),
    freeColumns() { }

AdjacencyMatrix::AdjacencyMatrix(unsigned long size) :
size(size), matrix(size, vector<bool>(size, false)), freeColumns(){}

unsigned long AdjacencyMatrix::totalSize() const {
    return size;
}

unsigned long AdjacencyMatrix::usedSize() const {
    return size - freeColumns.size();
}

void AdjacencyMatrix::addEdge(unsigned long a, unsigned long b) {
    checkArgument(a, "addEdge");
    checkArgument(b, "addEdge");
    matrix[a][b] = true;
}

void AdjacencyMatrix::removeEdge(unsigned long a, unsigned long b) {
    checkArgument(a, "removeEdge");
    checkArgument(b, "removeEdge");
    matrix[a][b] = false;
}

unsigned long AdjacencyMatrix::insertEntry() {
    if (freeColumns.empty()) {
        matrix.emplace_back(size, false);
        for (auto &list : matrix) {
            list.emplace_back(false);
        }
        size += 1;
        return size - 1;
    } else {
        unsigned long idx = freeColumns[0];
        freeColumns.erase(freeColumns.begin());
        return idx;
    }
}

void AdjacencyMatrix::deleteEntry(unsigned long a) {
    checkArgument(a, "deleteEntry");
    auto &list = matrix[a];
    fill(list.begin(), list.end(), false);

    for (auto &list_ : matrix) {
        list_[a] = false;
    }
    insert(freeColumns, a);
}

bool AdjacencyMatrix::reportEdge(unsigned long a, unsigned long b) const {
    checkArgument(a, "reportEdge");
    checkArgument(b, "reportEdge");
    return matrix[a][b];
}

const vector<unsigned long> AdjacencyMatrix::neighbours(unsigned long a) const {
    checkArgument(a, "neighbours");
    std::vector<unsigned long> result;
    for (unsigned long b = 0; b < size; b++) {
        result.push_back(b);
    }
    return result;
}

unsigned long AdjacencyMatrix::memoryUsage() {
    auto result = sizeof(AdjacencyMatrix);
    result += freeColumns.capacity() * sizeof(unsigned long);
    result += matrix.capacity() * sizeof(vector<bool>);
    for (const auto &list : matrix) {
        result += list.capacity() / 8;
    }
    return result;
}


AdjacencyMatrix::~AdjacencyMatrix() = default;

void AdjacencyMatrix::checkArgument(unsigned long k, std::string functionName) const {
    // Check that `k` is really a column/row in the graph
    if (k >= size) {
        std::stringstream error;
        error << functionName << ": invalid argument " << k << ", index out of range";
        throw std::invalid_argument(error.str());
    } else if (contains(freeColumns, k)) {
        std::stringstream error;
        error << functionName << ": invalid argument " << k << ", this column is unused";
        throw std::invalid_argument(error.str());
    }
}


