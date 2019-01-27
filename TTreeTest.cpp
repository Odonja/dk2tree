//
// Created by hugo on 2-1-19.
//

#ifndef TTREE_TEST
#define TTREE_TEST

#include "TTree.h"
#include <cstdio>
#include <iostream>

void print(TTree *tree, unsigned long depth = 0) {
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
            print(entry.P, depth + 1);
        }
    }
}

bool validate(TTree *tree) {
    if (tree->isLeaf) {
        return true;
    } else {
        auto &entries = tree->node.internalNode->entries;
        auto n = tree->node.internalNode->size;
        unsigned long i = 0;
        for (auto &entry : entries) {
            if (i < n) {
                if (!validate(entry.P)) {
                    return false;
                }
                EXPECT_NE(entry.P, nullptr);
                EXPECT_EQ(entry.b, entry.P->bits());
                EXPECT_EQ(entry.o, entry.P->ones());
                EXPECT_EQ(entry.P->parent, tree);
                EXPECT_EQ(entry.P->indexInParent, i);
                if (entry.P == nullptr
                    || entry.b != entry.P -> bits()
                    || entry.o != entry.P->ones()
                    || entry.P->parent != tree
                    || entry.P->indexInParent != i) {
                    return false;
                }
            } else {
                EXPECT_EQ(entry.P, nullptr);
                EXPECT_EQ(entry.b, 0);
                EXPECT_EQ(entry.o, 0);
                if (entry.P != nullptr
                    || entry.b != 0
                    || entry.o != 0) {
                    return false;
                }
            }
            i++;
        }
        return true;
    }
}

bool validateSize(TTree *tree) {
    if (tree == nullptr) {
        return true;
    }
    if (tree->isLeaf) {
        auto n = tree->size();
        if (tree->parent == nullptr) {
            EXPECT_LE(0, n);
            if (0 > n) {
                return false;
            }
        } else {
            EXPECT_LE(leafSizeMin, n);
            if (leafSizeMin > n) {
                return false;
            }
        }
        EXPECT_LE(n, leafSizeMax);
        if (n > leafSizeMax) {
            return false;
        }
    } else {
        auto &entries = tree->node.internalNode->entries;
        auto n = tree->node.internalNode->size;
        if (tree->parent == nullptr) {
            EXPECT_LE(2, n);
            if (2 > n) {
                return false;
            }
        } else {
            EXPECT_LE(nodeSizeMin, n);
            if (nodeSizeMin > n) {
                return false;
            }
        };
        EXPECT_LE(n, nodeSizeMax);
        if (n > nodeSizeMax) {
            return false;
        }
        for (auto &entry : entries) {
            if (!validateSize(entry.P)) {
                return false;
            }
        }
    }
    return true;
}

TEST(TTreeTest, AccessSetBit) {
    TTree node(20);

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

    ASSERT_TRUE(validate(&node));
}

TEST(TTreeTest, AccessSetBit2) {
    // Create example tree with 5 leaves (=2560 bits)
    auto *l1 = new TTree(B);
    auto *l2 = new TTree(B);
    auto *l3 = new TTree(B);
    auto *l4 = new TTree(B);
    auto *l5 = new TTree(B);
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

    ASSERT_TRUE(validate(root));

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
 * Tests the insert() and delete() functions
 */
TEST(TTreeTest, InsertDelete) {
    auto *root = new TTree(512);
    root->setBit(250, true);
    root->setBit(500, true);
    // Insert 50 bits at position 200, delete bits 450-499
    auto result = root->insertBits(200, 50);
    if (result != nullptr) {
        root = result;
    }
    // The rest has shifted right by 50 bits so we delete bits starting at 500
    result = root->deleteBits(500, 50);
    if (result != nullptr) {
        root = result;
    }

    ASSERT_TRUE(validate(root));

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
        auto *l1 = new TTree(512);
        auto *l2 = new TTree(512);
        auto *l3 = new TTree(512);
        auto *l4 = new TTree(512);
        auto *l5 = new TTree(512);
        auto *i4 = new TTree(l1, l2);
        auto *i3 = new TTree(l4, l5);
        auto *i2 = new TTree(i4, l3);
        auto *root = new TTree(i2, i3);

        delete root;
    }
}

TEST(TTreeTest, BPlusTest0) {
    auto *root = new TTree();
    ASSERT_TRUE(validate(root));
    ASSERT_TRUE(validateSize(root));
    unsigned long n = 512 * 100;
    vector<bool> bv(n, false);
    // Fill up the tree
    for (unsigned long i = 0; i < n; i += block) {
        auto result = root->insertBits(i, block);
        if (result != nullptr) {
            root = result;
        }
        ASSERT_TRUE(validate(root));
        ASSERT_TRUE(validateSize(root));
        printf("\n\n");
    }
    // Set some bits to 1
    for (unsigned long i = 0; i * i < n; i++) {
        bv[i * i] = true;
        bool changed = root->setBit(i * i, true);
        EXPECT_EQ(changed, true);
        ASSERT_TRUE(validate(root));
        ASSERT_TRUE(validateSize(root));
    }

    // EXPECT that the access() and rank() operations are the same
    // in the TTree as in the reference vector
    unsigned long rank = 0;
    for (unsigned long i = 0; i < n; i++) {
        EXPECT_EQ(root->access(i), bv[i]);
        EXPECT_EQ(root->rank1(i), rank);
        if (bv[i]) {
            rank += 1;
        }
    }
    EXPECT_EQ(root->rank1(n), rank);
}

#endif // TTREE_TEST