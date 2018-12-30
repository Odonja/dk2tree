#include <sdsl/suffix_arrays.hpp>
#include "TTree.h"

using namespace sdsl;

void printSome(TTree::TTreeNode &node, unsigned long lo, unsigned long hi) {
    for (unsigned long i = lo; i < hi; i++) {
        printf("bit %lu: %i\n", i, node.access(i));
    }
}

void accessSetTest() {
    TTree::TTreeNode node;

    printSome(node, 10, 20);
    node.setBit(13, true);
    node.setBit(17, true);
    printSome(node, 10, 20);

    long unsigned n;

    n = 10;
    printf("rank(%lu): %lu\n", n, node.rank1(n));

    n = 15;
    printf("rank(%lu): %lu\n", n, node.rank1(n));

    n = 20;
    printf("rank(%lu): %lu\n", n, node.rank1(n));

}

int main() {
    accessSetTest();
}
