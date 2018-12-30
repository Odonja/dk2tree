//
// Created by anneke on 18/12/18.
//

#include "TTree.h"

/**
 * Given an integer n, returns the child node containing the n-th bit in this subtree,
 *      as well as the numbers of bits and ones preceding it.
 * @param n  an integer that satisfies 0 <= n < (number of bits in this subtree)
 * @return  a TTree::InternalNode::Entry with:
 *      b = number of bits preceding this node in the TTree
 *      o = number of ones preceding this node in the TTree
 *      i = the index of the relevant subtree in the `entries` of `this`
 */
TTree::Record TTree::TTreeNode::findChild(unsigned long n) {
    if (this->isLeaf) {
        return {0, 0, 0};
    } else {
        unsigned long bitsBefore = 0;
        unsigned long onesBefore = 0;

        InternalNode node = this->node.internalNode;
        unsigned long i = 0;
        for (const auto &entry : node.entries) {
            if (bitsBefore + entry.b >= n) {
                return {bitsBefore, onesBefore, i};
            }
            bitsBefore += entry.b;
            onesBefore += entry.o;
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
TTree::InternalNode::Entry TTree::TTreeNode::findLeaf(unsigned long n) {
    auto *current = this;
    unsigned long bitsBefore = 0;
    unsigned long onesBefore = 0;
    while (!current->isLeaf) {
        auto record = current->findChild(n - bitsBefore);
        bitsBefore += record.b;
        onesBefore += record.o;
        current = current->node.internalNode.entries[record.i].P;
    }
    return {bitsBefore, onesBefore, current};
}

/**
 * Performs the `rank` operation on the bitvector represented by this tree
 * @param n  an integer with 0 <= n < (numbers of bits in the tree)
 * @return the number of 1-bits in the tree up to position n
 */
unsigned long TTree::TTreeNode::rank1(unsigned long n) {
    auto entry = findLeaf(n);
    return entry.o + entry.P -> node.leafNode.rs.rank(n - entry.b);
}

/**
 * Performs the `access` operation on this subtree
 * @param n the index of a bit in the `TTree`
 * @return the value of the `n`-th bit in the tree
 */
bool TTree::TTreeNode::access(unsigned long n) {
    auto entry = findLeaf(n);
    return entry.P -> node.leafNode.bv[n - entry.b];
}

/**
 * Sets the bit at position n to the value of b
 * @param n the index of the bit to set
 * @param b the value to set the bit to
 * @return true if this bit changed, e.g.
 *      if the previous value of the bit was unequal to b
 */
bool TTree::TTreeNode::setBit(unsigned long n, bool b) {
    if (this->isLeaf) {
        auto &bv = this->node.leafNode.bv;
        bool prev = bv[n];
        bv[n] = b;
        return prev ^ b;
    } else {
        auto record = this->findChild(n);
        auto *entry = &this->node.internalNode.entries[record.i];
        bool result = entry->P->setBit(n - record.b, b);
        if (result) {
            if (b) {
                entry->o += 1;
            } else {
                entry->o -= 1;
            }
        }
        return result;
    }
}