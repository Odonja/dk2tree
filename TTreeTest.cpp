//
// Created by hugo on 2-1-19.
//

#ifndef TTREE_TEST
#define TTREE_TEST

#include "TTree.h"
#include <cstdio>
#include <iostream>

TEST(TTreeTest, AccessSetBit) {
    TTree node;

    for (unsigned long i = 10; i < 20; i++) {
        EXPECT_FALSE(node.access(i)) << "Bit at position " << i << " is incorrectly set to 1";
    }

    node.setBit(13, true);
    node.setBit(17, true);
    for (unsigned long i = 10; i < 20; i++) {
        EXPECT_EQ(node.access(i), i == 13 || i == 17)
                            << "Bit at position " << i << " is incorrectly set to " << node.access(i);
    }

    EXPECT_EQ(node.rank1(10), 0);
    EXPECT_EQ(node.rank1(15), 1);
    EXPECT_EQ(node.rank1(20), 2);
}

unsigned long checkOnes(TTree *tree) {
    if (tree->isLeaf) {
        unsigned long n = tree->ones();
        return n;
    } else {
        auto &entries = tree->node.internalNode->entries;
        unsigned long n1 = checkOnes(entries[0].P);
        unsigned long n2 = checkOnes(entries[1].P);
        EXPECT_EQ(n1 + n2, tree->ones());

        return n1 + n2;
    }
}

unsigned long checkBits(TTree *tree) {
    if (tree->isLeaf) {
        unsigned long n = tree->bits();
        return n;
    } else {
        auto &entries = tree->node.internalNode->entries;
        unsigned long n1 = checkBits(entries[0].P);
        unsigned long n2 = checkBits(entries[1].P);
        EXPECT_EQ(n1 + n2, tree->bits());

        return n1 + n2;
    }
}

TEST(TTreeTest, AccessSetBit2) {
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
    // 100 randomly generated bits to flip with no consecutive entries
    unsigned long toFlip[]{
            5, 36, 60, 68, 98, 114, 116, 157, 226, 291,
            350, 411, 480, 491, 493, 504, 523, 540, 588, 596,
            616, 655, 662, 665, 676, 716, 724, 765, 793, 802,
            830, 848, 883, 915, 941, 966, 978, 1053, 1059, 1085,
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

    EXPECT_EQ(checkBits(root), 2560);
    EXPECT_EQ(checkOnes(root), 100);

    root->findLeaf(0).P->split();

    EXPECT_EQ(checkBits(root), 2560);
    EXPECT_EQ(checkOnes(root), 100);

    // Check that access and rank operations return the correct results
    unsigned long idx = 0;
    for (long unsigned i = 0; i < 2560; i++) {
        bool ac, ac2 = false;
        unsigned long rk = root->rank1(i), rk2 = idx;
        ac = root->access(i);
        if (i == toFlip[idx]) {
            ac2 = true;
            idx++;
        }
        EXPECT_EQ(ac, ac2);
        EXPECT_EQ(rk, rk2);
    }
}

/**
 * Tests the split() function on the root of the tree
 */
TEST(TTreeTest, SplitRoot) {
    auto *root = new TTree;
    root->setBit(250, true);
    root->setBit(300, true);
    root->split();

    for (unsigned long i = 0; i < 512; i++) {
        EXPECT_EQ(root->access(i), i == 250 || i == 300) << "Bit at position " << i << " is incorrectly set to 1";
    }
}

/**
 * Tests the insert() and delete() functions
 */
TEST(TTreeTest, InsertDelete) {
    auto *root = new TTree;
    root->split();
    root->setBit(250, true);
    root->setBit(500, true);
    // Insert 50 bits at position 200, delete bits 450-499
    root->insertBits(200, 50);
    // The rest has shifted right by 50 bits so we delete bits starting at 500
    root->deleteBits(500, 50);

    // Exactly bits 300 and 500 should be true
    EXPECT_EQ(root->bits(), 512);
    EXPECT_EQ(root->ones(), 2);

    for (unsigned long i = 0; i < 512; i++) {
        EXPECT_EQ(root->access(i), i == 300 || i == 500);
    }
}

/**
 * Creates the tree from `largerAccessSetTest` a million times and destroys it
 * immediately. Ubuntu's resource monitor indicates that the memory usage is
 * constant (instead of rising steadily), indicating that there is no memory leak.
 */
void treeTestMemoryLeaking() {
    for (int i = 1; i <= 1000000; i++) {
        if (i % 1000 == 0) {
            std::cout << i << std::endl;
        }
        auto *l1 = new TTree;
        auto *l2 = new TTree;
        auto *l3 = new TTree;
        auto *l4 = new TTree;
        auto *l5 = new TTree;
        auto *i4 = new TTree(l1, l2);
        auto *i3 = new TTree(l4, l5);
        auto *i2 = new TTree(i4, l3);
        auto *root = new TTree(i2, i3);

        root->findLeaf(0).P->split();

        delete root;
    }
}

/**
 * Runs all the different tests except for the memory test (which takes very long)
 */
int treeTestAll(int argc, char **argv) {
    return 0;
}

#endif // TTREE_TEST