#include "TTree.h"
#include <stdio.h>
#include <assert.h>

void printSome(TTree &node, unsigned long lo, unsigned long hi) {
    printf("  bits [%lu...%lu] = [%i", lo, hi, node.access(lo));
    for (unsigned long i = lo + 1; i < hi; i++) {
        printf(", %i", node.access(i));
    }
    printf("]\n");
}

void accessSetTest() {
    printf("Testing access+setbit+rank1 on leaf\n");
    TTree node;

    printSome(node, 10, 20);
    node.setBit(13, true);
    node.setBit(17, true);
    printSome(node, 10, 20);

    long unsigned n;

    n = 10;
    printf("  rank(%lu): %lu\n", n, node.rank1(n));

    n = 15;
    printf("  rank(%lu): %lu\n", n, node.rank1(n));

    n = 20;
    printf("  rank(%lu): %lu\n", n, node.rank1(n));
}

unsigned long printOnes(TTree *tree) {
    if (tree -> isLeaf) {
        unsigned long n = tree->rank1(512);
        printf("%lu", n);
        return n;
    } else {
        auto &entries = tree->node.internalNode->entries;
        printf("(");
        unsigned long n1 = printOnes(entries[0].P);
        printf(" + ");
        unsigned long n2 = printOnes(entries[1].P);
        printf(" = %lu)", n1 + n2);

        return n1 + n2;
    }
}

void largerAccessSetTest() {
    printf("Testing access+setbit+rank1 on larger tree\n");
    // Create example tree with 5 leaves (=2560 bits)
    auto *l1 = new TTree;
    auto *l2 = new TTree;
    auto *l3 = new TTree;
    auto *l4 = new TTree;
    auto *l5 = new TTree;
    auto *i4 = new TTree(l1, l2);
    auto *i3 = new TTree(l4, l5);
    auto *i2 = new TTree(i4, l3);
    auto *root = new TTree(i2, i3);
    printf("  Total bits: %lu\n", root->bits());

    // 100 randomly generated bits to flip with no consecutive entries
    unsigned long toFlip[]{
           5,   36,   60,   68,   98,  114,  116,  157,  226,  291,
         350,  411,  480,  491,  493,  504,  523,  540,  588,  596,
         616,  655,  662,  665,  676,  716,  724,  765,  793,  802,
         830,  848,  883,  915,  941,  966,  978, 1053, 1059, 1085,
        1095, 1166, 1203, 1217, 1227, 1239, 1259, 1262, 1373, 1399,
        1408, 1419, 1441, 1462, 1464, 1470, 1480, 1502, 1506, 1520,
        1597, 1687, 1690, 1700, 1762, 1766, 1773, 1790, 1810, 1827,
        1874, 1893, 1903, 1907, 1923, 1967, 1985, 1996, 2003, 2023,
        2026, 2028, 2172, 2184, 2233, 2257, 2330, 2353, 2375, 2405,
        2410, 2416, 2425, 2427, 2437, 2446, 2495, 2511, 2514, 2517
    };
    for (auto i : toFlip) {
        root->setBit(i, true);
    }

    printf("  Total ones: ");
    printOnes(root);
    printf("\n");

    // Check that access and rank operations return the correct results
    unsigned long idx = 0;
    for (long unsigned i = 0; i < 2560; i++) {
        bool ac, ac2 = false;
        unsigned long rk = root->rank1(i), rk2 = idx;
        ac = root->access(i);
        if (i == toFlip[idx]) {
            ac2 = true;
            idx ++;
        }
        assert(ac == ac2);
        assert(rk == rk2);
    }
}

int main() {
    accessSetTest();
    largerAccessSetTest();
}
