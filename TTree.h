//
// Created by anneke on 18/12/18.
//

#ifndef DK2TREE_TTREE_H
#define DK2TREE_TTREE_H

#include "BitVector.h"
#include <utility>

// The three main parameters for the TTree and LTree representation
static const unsigned int k = 2; // The `k` in the k2-tree
static const unsigned int block = k * k; // The number of bits in one block of the bit vector
static const unsigned int B = 512; // The maximum size (in bits) of a leaf bitvector

// The maximum/minimum number of children/blocks an internal node/leaf node
// is allowed to have, as per the rules of the B+tree
static const unsigned int nodeSizeMax = 31;
static const unsigned int nodeSizeMin = (nodeSizeMax + 1) / 2;
static const unsigned int leafSizeMax = B / block;
static const unsigned int leafSizeMin = (leafSizeMax + 1) / 2;

/// Record type containing the number pf preceding bits and ones, and the
/// index of a child node in the parent's `entries` list
struct Record {
    unsigned long b;
    unsigned long o;
    unsigned long i;

    Record(unsigned long b, unsigned long o, unsigned long i) :
            b(b), o(o), i(i) {}
};

struct InternalNode;
struct LeafNode;
struct TTree;

/**
 * A struct for the internal nodes of the tree
 * This contains a list of Entries of the form <b, o, P>
 */
struct InternalNode {
    struct Entry {
        unsigned long b;
        unsigned long o;
        TTree *P;

        Entry() :
                b(0), o(0), P(nullptr) {}

        explicit Entry(TTree *P);

        Entry(unsigned long b, unsigned long o, TTree *P) :
                b(b), o(o), P(P) {}

        /**
         * This function is called in the destructor of `InternalNode`, to
         * delete the child nodes. It is not part of the destructor of `Entry`,
         * since that causes problems when the struct is used in methods such as
         * `findLeaf`
         */
        void remove();
    };

    /// The number of children this node has
    unsigned long size;

    /// An array of pointers to the child nodes
    /// This is one more than the maximum, so that we can split nodes
    /// after insertion instead of before
    Entry entries[nodeSizeMax + 1];

    InternalNode() :
            size(0),
            entries{Entry()} {}

    InternalNode(TTree *left, TTree *right, TTree *parent = nullptr);

    ~InternalNode() {
        for (auto &entry : entries) {
            entry.remove();
        }
    }

    unsigned long bits();

    unsigned long ones();

    Entry popFirst();

    Entry popLast();

    void insert(unsigned long, Entry);

    void append(Entry);

    void remove(unsigned long);
};

/** A leaf node, which consists of a bitvector (represented by vector<bool>) */
struct LeafNode {
    BitVector bv;

    /// Constructs a leaf with the given number of bits
    explicit LeafNode(unsigned long size) :
            bv(size) {}

    /// Constructs a leaf node from the given bit vector
    explicit LeafNode(BitVector bv) :
            bv(std::move(bv)) {}

    unsigned long bits();

    unsigned long ones();
};

/** TTree is the struct representing a single node (leaf or internal) of the TTree */
/** A single node is either an internal or leaf node, as indicated by the isLeaf value */
struct TTree {
    bool isLeaf;
    TTree *parent = nullptr;
    unsigned long indexInParent = 0;

    union Node {
        InternalNode *internalNode;
        LeafNode *leafNode;

        Node();

        Node(TTree *, TTree *);

        explicit Node(BitVector);

        explicit Node(unsigned long);
    } node;

    TTree() :
            isLeaf(true),
            node() {}

    TTree(TTree *left, TTree *right) :
            isLeaf(false),
            node(left, right) {
        left->parent = this;
        left->indexInParent = 0;
        right->parent = this;
        right->indexInParent = 1;
    }

    explicit TTree(BitVector bv) :
            isLeaf(true),
            node(std::move(bv)) {}

    explicit TTree(unsigned long size) :
            isLeaf(true),
            node(size) {}

    /// The TTree destructor decides which variant of the union to destroy
    ~TTree();

    /**
 * Returns the depth of this node, which is the length of the path from this
 * node to the root
 * @return the depth of `this`
 */
    unsigned long depth();

    /**
 * Returns the height of this node, which is the length of the longest path from
 * this node to any leaf in its subtree.
 * @return the height of `this`
 */
    unsigned long height();

    unsigned long size();

    /**
 * Given an integer n, returns the child node containing the n-th bit in this subtree,
 *      as well as the numbers of bits and ones preceding it.
 * @param n  an integer that satisfies 0 <= n < (number of bits in this subtree)
 * @return  a TTree::InternalNode::Entry with:
 *      b = number of bits preceding this node in the TTree
 *      o = number of ones preceding this node in the TTree
 *      i = the index of the relevant subtree in the `entries` of `this`
 */
    Record findChild(unsigned long);

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
    InternalNode::Entry findLeaf(unsigned long);

    /**
 * Performs the `rank` operation on the bitvector represented by this tree
 * @param n  an integer with 0 <= n < (numbers of bits in the tree)
 * @return the number of 1-bits in the tree up to position n
 */
    unsigned long rank1(unsigned long);

    /**
 * Performs the `access` operation on this subtree
 * @param n the index of a bit in the `TTree`
 * @return the value of the `n`-th bit in the tree
 */
    bool access(unsigned long);

    /**
 * Sets the bit at position n to the value of b
 * @param n the index of the bit to set
 * @param b the value to set the bit to
 * @return true if this bit changed, e.g.
 *      if the previous value of the bit was unequal to b
 */
    bool setBit(unsigned long, bool);

    unsigned long bits();

    unsigned long ones();

    /**
 * Changes the values of the counters `b` and `o` of all the nodes whose
 * subtree contains this node. Used to update these counters after modifying
 * the underlying bitvector, or the structure of the tree.
 *
 * @param dBits the change in the number of bits (e.g. 4 when 4 bits were inserted)
 * @param dOnes the change in the number of ones (e.g. 2 when 2 zeros were set to ones)
 */
    void updateCounters(long, long);

    /**
 * Inserts the given number of bits (set to zero) at the given position in the tree
 *
 * @param index the position at which to insert bits
 * @param count the number of bits to insert
     *
     * @return nullptr in most cases, but a pointer to the new root node
     * if this insert operation created a new root (e.g. when the height of the
     * tree increases)
 */
    TTree *insertBits(long unsigned, long unsigned);

    /**
 * Deletes the given number of bits in the subtree, assuming they are all in the
 * same leaf node. This will be the case if a group of k^2 bits are deleted
 *
 * @param index the position of the first bit to delete
 * @param count the number of bits to delete
     *
     * @return nullptr in most cases, but a pointer to the new root node
     * if this insert operation changed the root (e.g. when the height of the
     * tree decreases)
 */
    TTree *deleteBits(long unsigned, long unsigned);

    TTree *checkSizeUpper();

    TTree *checkSizeLower();

    bool trySpillInternal();

    bool trySpillLeaf();

    TTree *splitInternal();

    TTree *splitLeaf();

    bool tryStealInternal();

    bool tryStealLeaf();

    TTree *mergeInternal();

    TTree *mergeLeaf();

    void moveLeftInternal();

    void moveRightInternal();

    void moveLeftLeaf();

    void moveRightLeaf();
};

#endif //DK2TREE_TTREE_H
