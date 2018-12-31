//
// Created by anneke on 18/12/18.
//

#include "TTree.h"
#include <sdsl/util.hpp>

TTree::Node::Node() {
    this->internalNode = nullptr;
    this->leafNode = new LeafNode(B);
}

TTree::Node::Node(TTree *P1, TTree *P2) {
    this->leafNode = nullptr;
    this->internalNode = new InternalNode();
    internalNode->entries[0] = InternalNode::Entry(P1);
    internalNode->entries[1] = InternalNode::Entry(P2);
}

TTree::~TTree() {
    if (isLeaf) {
        node.leafNode->~LeafNode();
    } else {
        node.internalNode->~InternalNode();
    }
}

InternalNode::Entry::Entry(TTree *P) {
    b = P->bits();
    o = P->ones();
    this->P = P;
}

void InternalNode::Entry::remove() {
    delete P;
}

/**
 * Counts the number of one-bits in the specified range in the bit vector
 * TODO try optimising by counting per byte
 *
 * @param bv a vector<bool>
 * @param lo an integer with 0 <= lo <= bv.size()
 * @param hi an integer with lo <= hi <= bv.size()
 * @return the number of one-bits in the bits bv[lo] .. bv[hi - 1]
 */
unsigned long countOnes(const bit_vector &bv, unsigned long lo, unsigned long hi) {
    unsigned long tot = 0;
    for (unsigned long i = lo; i < hi; i++) {
        if (bv[i]) {
            tot ++;
        }
    }
    return tot;
}

/**
 * Given an integer n, returns the child node containing the n-th bit in this subtree,
 *      as well as the numbers of bits and ones preceding it.
 * @param n  an integer that satisfies 0 <= n < (number of bits in this subtree)
 * @return  a TTree::InternalNode::Entry with:
 *      b = number of bits preceding this node in the TTree
 *      o = number of ones preceding this node in the TTree
 *      i = the index of the relevant subtree in the `entries` of `this`
 */
Record TTree::findChild(unsigned long n) {
    if (this->isLeaf) {
        return {0, 0, 0};
    } else {
        unsigned long bitsBefore = 0;
        unsigned long onesBefore = 0;

        InternalNode *node = this->node.internalNode;
        unsigned long i = 0;
        for (const auto &entry : node->entries) {
            if (bitsBefore + entry.b > n) {
                return {bitsBefore, onesBefore, i};
            }
            bitsBefore += entry.b;
            onesBefore += entry.o;
            i += 1;
        }
    }
    // If we reach this point, that means that the size of this subtree is less than n
    // so the input parameter was out of range
    // TODO throw exception or something
    std::cout << "Oh nee!" << std::endl;
}

/**
 * Given an integer n, returns the leaf containing the n-th bit of the bitvector,
 *      as well as the numbers of bits and ones preceding this leaf node.
 * @param n  an integer that satisfies 0 <= n < (number of bits in this subtree)
 * @return  a TTree::InternalNode::Entry with:
 *      b = number of bits preceding this node in the TTree
 *      o = number of ones preceding this node in the TTree
 *      P = a pointer to the leaf node containing the n-th bit
 * Note that P is strictly just a pointer to a `TTreeNode`, as defined by the `entry` struct
 * But the union is always of the `LeafNode` variant.
 */
InternalNode::Entry TTree::findLeaf(unsigned long n) {
    auto *current = this;
    unsigned long bitsBefore = 0;
    unsigned long onesBefore = 0;
    while (!current->isLeaf) {
        auto record = current->findChild(n - bitsBefore);
        bitsBefore += record.b;
        onesBefore += record.o;
        current = current->node.internalNode->entries[record.i].P;
    }
    return {bitsBefore, onesBefore, current};
}

/**
 * Performs the `rank` operation on the bitvector represented by this tree
 * @param n  an integer with 0 <= n < (numbers of bits in the tree)
 * @return the number of 1-bits in the tree up to position n
 */
unsigned long TTree::rank1(unsigned long n) {
    auto entry = findLeaf(n);
    auto &bv = entry.P->node.leafNode->bv;
    return entry.o + countOnes(bv, 0, n - entry.b);
}

/**
 * Performs the `access` operation on this subtree
 * @param n the index of a bit in the `TTree`
 * @return the value of the `n`-th bit in the tree
 */
bool TTree::access(unsigned long n) {
    auto entry = findLeaf(n);
    return entry.P -> node.leafNode->bv[n - entry.b];
}

/**
 * Sets the bit at position n to the value of b
 * @param n the index of the bit to set
 * @param b the value to set the bit to
 * @return true if this bit changed, e.g.
 *      if the previous value of the bit was unequal to b
 */
bool TTree::setBit(unsigned long n, bool b) {
    // Find the leaf node that contains this bit
    auto entry = findLeaf(n);
    bit_vector &bv = entry.P->node.leafNode->bv;
    bool changed = bv[n - entry.b] ^ b;
    bv[n - entry.b] = b;

    if (changed) {
        // Change the one-counters all the way up from this leaf
        entry.P->updateCounters(0, b ? 1ul : -1ul);
    }
    return changed;
}

/**
 * Changes the values of the counters `b` and `o` of all the nodes whose
 * subtree contains this node. Used to update these counters after modifying
 * the underlying bitvector, or the structure of the tree.
 *
 * @param dBits the change in the number of bits (e.g. 4 when 4 bits were inserted)
 * @param dOnes the change in the number of ones (e.g. 2 when 2 zeros were set to ones)
 */
void TTree::updateCounters(long dBits, long dOnes) {
    TTree *current = this;
    // Go up in the tree until we reach the root
    while (current->parent != nullptr) {
        // Take the entry in `current`s parent that points to `current`,
        // and update its `b` and `o` counters.
        auto parent = current->parent;
        auto &entry = parent->node.internalNode->entries[current->indexInParent];
        entry.b += dBits;
        entry.o += dOnes;

        current = parent;
    }
}

/// Methods for determining the number of bits and ones in a leaf or internal node
unsigned long TTree::bits() {
    if (isLeaf) {
        return node.leafNode->bits();
    } else {
        return node.internalNode->bits();
    }
}

unsigned long TTree::ones() {
    if (isLeaf) {
        return node.leafNode->ones();
    } else {
        return node.internalNode->ones();
    }
}

unsigned long InternalNode::bits() {
    unsigned long total = 0;
    for (const auto &entry : entries) {
        total += entry.b;
    }
    return total;
}

unsigned long InternalNode::ones() {
    unsigned long total = 0;
    for (const auto &entry : entries) {
        total += entry.o;
    }
    return total;
}

unsigned long LeafNode::bits() {
    return bv.size();
}

unsigned long LeafNode::ones() {
    // Count all ones manually
    return countOnes(this->bv, 0, B);
}
