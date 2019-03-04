//
// Created by hugo on 01/03/19.
//

#include <fstream>
#include "DKTree.h"

using std::string;
using std::ifstream;
using std::cin;

/**
 * Reads an edge-list file with the given name and loads a DKTree from it
 * @param name the location of the file to load
 * @return a DKTree containing exactly the edges specified by the file
 */
DKTree *makeGraphFromFile(const string &name, bool verbose = false) {
    ifstream file(name);
    auto tree = new DKTree();
    unsigned long size = 0;
    unsigned long n = 1;

    // Read the two integer from each line, as long as that is possible
    unsigned long a, b;
    while (file >> a >> b) {
        if (verbose && n % 10000 == 0) {
            std::cout << n << '\r';
            std::cout.flush();
        }
        n++;

        // Make sure that the tree has nodes for a and b, so that we can connect
        // them properly
        while (a >= size || b >= size) {
            tree->insertEntry();
            size++;
        }
        // Then add the edge
        tree->addEdge(a, b);
    }
    return tree;
}