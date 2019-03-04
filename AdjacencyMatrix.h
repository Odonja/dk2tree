//
// Created by hugo on 14-2-19.
//

#ifndef DK2TREE_ADJACENCYMATRIX_H
#define DK2TREE_ADJACENCYMATRIX_H

#include <vector>
#include <string>

using namespace std;

struct AdjacencyMatrix {
    unsigned long size;
    vector<vector<bool>> matrix;
    vector<unsigned long> freeColumns;

public:
    AdjacencyMatrix();

    explicit AdjacencyMatrix(unsigned long size);

    ~AdjacencyMatrix();

    unsigned long totalSize() const;

    unsigned long usedSize() const;

    void addEdge(unsigned long a, unsigned long b);

    void removeEdge(unsigned long a, unsigned long b);

    unsigned long insertEntry();

    void deleteEntry(unsigned long a);

    bool reportEdge(unsigned long a, unsigned long b) const;

    const vector<unsigned long> neighbours(unsigned long a) const;

    unsigned long memoryUsage();

    static AdjacencyMatrix *withSize(unsigned long size) {
        return new AdjacencyMatrix(size);
    }

private:
    void checkArgument(unsigned long k, string functionName) const;
};


#endif //DK2TREE_ADJACENCYMATRIX_H
