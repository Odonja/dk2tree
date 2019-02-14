//
// Created by hugo on 9-2-19.
//

#include "AdjacencyLists.h"
#include <sstream>
#include "BinarySearch.h"

AdjacencyLists::AdjacencyLists() :
    size(0),
    lists(),
    freeColumns() { }

AdjacencyLists::AdjacencyLists(unsigned long size) :
    size(size),
    lists(size),
    freeColumns() { }

AdjacencyLists::~AdjacencyLists() = default;

unsigned long AdjacencyLists::totalSize() const {
    return size;
}

unsigned long AdjacencyLists::usedSize() const {
    return size - freeColumns.size();
}

void AdjacencyLists::addEdge(unsigned long a, unsigned long b) {
    checkArgument(a, "addEdge");
    checkArgument(b, "addEdge");
    insert(lists[a], b);
}

void AdjacencyLists::removeEdge(unsigned long a, unsigned long b) {
    checkArgument(a, "addEdge");
    checkArgument(b, "addEdge");
    remove(lists[a], b);
}

unsigned long AdjacencyLists::insertEntry() {
    if (freeColumns.empty()) {
        size += 1;
        lists.emplace_back();
        return size - 1;
    } else {
        unsigned long idx = freeColumns[0];
        freeColumns.erase(freeColumns.begin());
        return idx;
    }
}

void AdjacencyLists::deleteEntry(unsigned long a) {
    checkArgument(a, "deleteEntry");
    lists[a].clear();
    for (vector<unsigned long> &v : lists) {
        remove(v, a);
    }
    insert(freeColumns, a);
}

bool AdjacencyLists::reportEdge(unsigned long a, unsigned long b) const {
    checkArgument(a, "reportEdge");
    checkArgument(b, "reportEdge");
    return contains(lists[a], b);
}

void AdjacencyLists::checkArgument(unsigned long k, std::string functionName) const {
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

const vector<unsigned long> AdjacencyLists::neighbours(unsigned long a) const {
    checkArgument(a, "neighbours");
    return lists[a];
}

unsigned long AdjacencyLists::memoryUsage() {
    unsigned long result = sizeof(AdjacencyLists);
    result += lists.size() * sizeof(vector<unsigned long>);
    result += freeColumns.size() * sizeof(unsigned long);
    for (auto &v : lists) {
        result += v.size() * sizeof(unsigned long);
    }
    return result;
}
