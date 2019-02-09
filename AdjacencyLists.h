//
// Created by hugo on 9-2-19.
//

#ifndef DK2TREE_ADJACENCYLISTS_H
#define DK2TREE_ADJACENCYLISTS_H

#include <vector>
#include <string>

using namespace std;

class AdjacencyLists {
private:
    unsigned long size;
    vector<vector<unsigned long>> lists;
    vector<unsigned long> freeColumns;

public:
    AdjacencyLists();

    explicit AdjacencyLists(unsigned long size);

    ~AdjacencyLists();

    unsigned long totalSize();

    unsigned long usedSize();

    void addEdge(unsigned long a, unsigned long b);

    void removeEdge(unsigned long a, unsigned long b);

    unsigned long insertEntry();

    void deleteEntry(unsigned long a);

    bool reportEdge(unsigned long a, unsigned long b);

private:
    void checkArgument(unsigned long k, string functionName);
};


#endif //DK2TREE_ADJACENCYLISTS_H
