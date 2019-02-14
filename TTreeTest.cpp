//
// Created by hugo on 2-1-19.
//

#ifndef TTREE_TEST
#define TTREE_TEST

#include "TTree.h"
#include <cstdio>
#include <iostream>
#include "gtest/gtest.h"

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

/**
 * Checks that the tree given by `tree` is a valid tree, meaning for internal nodes:
 *   - The `size` is equal to the number of non-null entries
 *   - Each entry's b- and o-counters equal the number of bits/ones in that subtree
 *   - Each entry's subtree has correct `parent` and `indexInParent`
 * @param tree the tree to be validated
 * @return true if and only if the above criteria are met
 * Note that the use of EXPECT_* from Googletest is coupled with lots of `if (...) return false`
 * This is because ASSERT_* statements only exit the current function when failed, which is not useful for
 * recursive methods
 */
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

/**
 * Checks that the tree given by `tree` is a valid B+ tree, meaning that each node satisfies the relevant size constraints
 *
 * @param tree the tree to validate the node sizes of
 */
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

/**
 * Compares a TTree against a vector<bool>, to check if the two contain the same data and that the tree's access() and
 * rank1() operations are correct regarding the data it contains
 */
bool treeEqualsVec(TTree *root, vector<bool> bv, unsigned long lo = 0, unsigned long hi = ~0UL) {
    unsigned long n1 = root->bits();
    if (hi == ~0) {
        hi = bv.size();
    }
    unsigned long n2 = hi - lo;
    EXPECT_EQ(n1, n2);
    if (n1 != n2) {
        return false;
    }

    // EXPECT that the access() and rank() operations are the same
    // in the TTree as in the reference vector
    unsigned long rank = 0;
    for (unsigned long i = 0; i < n1; i++) {
        bool b1 = root->access(i);
        bool b2 = bv[i + lo];
        EXPECT_EQ(b1, b2);
        if (b1 != b2) {
            return false;
        }
        unsigned long r1 = root->rank1(i);
        EXPECT_EQ(r1, rank);
        if (r1 != rank) {
            return false;
        }
        if (bv[i + lo]) {
            rank += 1;
        }
    }
    unsigned long r1 = root->rank1(n1);
    EXPECT_EQ(r1, rank);
    return r1 == rank;
}

/**
 * Inserts a block into the tree and updates the `root` variable automatically
 */
void insertBlock(TTree **root, unsigned long position) {
    auto result = (*root)->insertBlock(position);
    if (result != nullptr) {
        *root = result;
    }
}

/**
 * Deletes a block from the tree and updates the `root` variable automatically
 */
void deleteBlock(TTree **root, unsigned long position) {
    auto result = (*root)->deleteBlock(position);
    if (result != nullptr) {
        *root = result;
    }
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
    // Create example tree with 5 leaves and 2560 bits
    // Note that this test creates a binary tree which may not be a valid B+Tree
    // However, no insertions/deletions are done so the B+Tree-specific code is never executed
    auto *l1 = new TTree(512);
    auto *l2 = new TTree(512);
    auto *l3 = new TTree(512);
    auto *l4 = new TTree(512);
    auto *l5 = new TTree(512);
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
    ASSERT_EQ(root->bits(), 2560);
    ASSERT_EQ(root->ones(), 100);

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
    // This test assumes that B is not too small. Else, we skip this test
    if (B <= 2 * block) {
        printf("B = %u and block = %u means skipping TTreeTest::InsertDelete\n", B, block);
        return;
    }

    auto *root = new TTree(B);
    root->setBit(B / 2 - 1, true);
    root->setBit(B / 2, true);
    root->setBit(B - 1, true);
    // tree = 0000 ... 0001 1000 ... 0000 0001
    insertBlock(&root, B / 2);
    // tree = 0000 ... 0001 0000 1000 ... 0000 0001
    deleteBlock(&root, B);
    // tree = 0000 ... 0001 0000 1000 ... 0000

    ASSERT_TRUE(validate(root));

    // Exactly bits 300 and 500 should be true
    EXPECT_EQ(root->bits(), B);
    EXPECT_EQ(root->ones(), 2);

    for (unsigned long i = 0; i < B; i++) {
        EXPECT_EQ(root->access(i), i == B / 2 - 1 || i == B / 2 + block);
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
    unsigned long checkInterval = 100;
    vector<bool> bv(n, false);
    // Fill up the tree
    for (unsigned long i = 0; i < n; i += block) {
        insertBlock(&root, i);
        if ((i + 1) % checkInterval == 0) {
            ASSERT_TRUE(validate(root));
            ASSERT_TRUE(validateSize(root));
        }
    }
    // Set some bits to 1
    for (unsigned long i = 0; i * i < n; i++) {
        bv[i * i] = true;
        bool changed = root->setBit(i * i, true);
        EXPECT_EQ(changed, true);
    }

    printf("size: %lu\n", root->memoryUsage());

    ASSERT_TRUE(validate(root));
    ASSERT_TRUE(validateSize(root));
    ASSERT_TRUE(treeEqualsVec(root, bv));
}

TEST(TTreeTest, BPlusTest1) {
    // Insert blocks at the end, then remove from the front
    // All the while, the tree and bit vector should remain equal
    unsigned long numBlocks = 1024;
    unsigned long checkInterval = 32;
    unsigned long totalSize = block * numBlocks;
    auto *root = new TTree();
    vector<bool> ref(totalSize, false);
    unsigned long lo = 0, hi = 0;

    for (unsigned long i = 0; i < numBlocks; i++) {
        insertBlock(&root, i * block);
        if (i % 17 == 0) {
            for (unsigned long j = 0; j < block; j++) {
                bool changed = root->setBit(i * block + j, true);
                ASSERT_TRUE(changed);
                ref[i * block + j] = true;
            }
        }
        hi += block;
        if ((i + 1) % checkInterval == 0) {
            ASSERT_TRUE(validate(root));
            ASSERT_TRUE(validateSize(root));
            ASSERT_TRUE(treeEqualsVec(root, ref, lo, hi));
        }
    }

    printf("size: %lu\n", root->memoryUsage());

    ASSERT_TRUE(treeEqualsVec(root, ref));

    // Delete everything again
    for (unsigned long i = 0; i < numBlocks; i++) {
        deleteBlock(&root, 0);
        lo += block;
        if ((i + 1) % checkInterval == 0) {
            ASSERT_TRUE(validate(root));
            ASSERT_TRUE(validateSize(root));
            ASSERT_TRUE(treeEqualsVec(root, ref, lo, hi));
        }
    }
}

TEST(TTreeTest, BPlusTest2) {
    // Insert blocks at the start, then remove from the end
    // All the while, the tree and bit vector should remain equal
    unsigned long numBlocks = 1024;
    unsigned long checkInterval = 32;
    unsigned long totalSize = block * numBlocks;
    auto *root = new TTree();
    vector<bool> ref(totalSize, false);
    unsigned long lo = totalSize, hi = totalSize;

    for (unsigned long i = 0; i < numBlocks; i++) {
        insertBlock(&root, 0);
        if (i % 17 == 0) {
            for (unsigned long j = 0; j < block; j++) {
                bool changed = root->setBit(j, true);
                ASSERT_TRUE(changed);
                ref[(numBlocks - 1 - i) * block + j] = true;
            }
        }
        lo -= block;
        if ((i + 1) % checkInterval == 0) {
            ASSERT_TRUE(validate(root));
            ASSERT_TRUE(validateSize(root));
            ASSERT_TRUE(treeEqualsVec(root, ref, lo, hi));
        }
    }

    printf("size: %lu\n", root->memoryUsage());

    ASSERT_TRUE(treeEqualsVec(root, ref));

    // Delete everything again
    for (unsigned long i = numBlocks; i > 0; i--) {
        deleteBlock(&root, (i - 1) * block);
        hi -= block;
        if ((i - 1) % checkInterval == 0) {
            ASSERT_TRUE(validate(root));
            ASSERT_TRUE(validateSize(root));
            ASSERT_TRUE(treeEqualsVec(root, ref, lo, hi));
        }
    }
}

TEST(TTreeTest, BPlusTest3) {
    // 1. Create `n` leaves worth of ones
    // 2. Insert empty blocks in between all of them
    // 3. Delete the ones
    unsigned long n = 4;
    unsigned long numBlocks = n * leafSizeMax;
    auto root = new TTree();
    for (unsigned long i = 0; i < numBlocks; i++) {
        insertBlock(&root, 0);
        for (unsigned long j = 0; j < block; j++) {
            root->setBit(j, true);
        }
    }
    // tree = 1111...1111

    // Make sure that the tree consists of the correct number of ones
    unsigned long len = numBlocks * block;
    ASSERT_EQ(root->bits(), len);
    for (unsigned long i = 0; i < len; i++) {
        ASSERT_TRUE(root->access(i));
        ASSERT_EQ(root->rank1(i), i);
    }
    ASSERT_EQ(root->rank1(len), len);

    for (unsigned long i = 0; i < numBlocks; i++) {
        insertBlock(&root, 2 * i * block);
    }
    // *tree = 000011110000...00001111

    // Create the reference vector and test it against the tree
    vector<bool> ref(2 * len);
    for (unsigned long i = block; i < 2 * len; i += 2 * block) {
        for (unsigned long j = 0; j < block; j++) {
            ref[i + j] = true;
        }
    }

    printf("size: %lu\n", root->memoryUsage());

    ASSERT_TRUE(treeEqualsVec(root, ref));

    for (unsigned long i = block; i <= len; i += block) {
        deleteBlock(&root, i);
    }

    ASSERT_EQ(root->bits(), len);
    for (unsigned long i = 0; i < len; i++) {
        ASSERT_FALSE(root->access(i));
        ASSERT_EQ(root->rank1(i), 0);
    }
}

#endif // TTREE_TEST