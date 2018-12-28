//
// Created by anneke on 18/12/18.
//

#include "TTree.h"

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
    if (this->isLeaf) {
        return {0, 0, this};
    } else {
        unsigned long bitsBefore = 0;
        unsigned long onesBefore = 0;

        InternalNode node = this->node.internalNode;
        for (auto entry : node.entries) {
            if (bitsBefore + entry.b >= n) {
                return {bitsBefore, onesBefore, entry.P};
            }
            bitsBefore += entry.b;
            onesBefore += entry.o;
        }
    }
    // TODO throw exception or something
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