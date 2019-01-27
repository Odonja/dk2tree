//
// Created by anneke on 18/12/18.
//

#ifndef DK2TREE_TTREE_H
#define DK2TREE_TTREE_H

#include "BitVector.h"
#include <utility>

// The three main parameters for the TTree and LTree representation
static const unsigned int k = 2; // The `k` in the k2-tree
static const unsigned int block = k*k; // The number of bits in one block of the bit vector
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
            entries{Entry()}{}

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

    unsigned long depth();

    unsigned long height();

    unsigned long size();

    Record findChild(unsigned long);

    InternalNode::Entry findLeaf(unsigned long);

    unsigned long rank1(unsigned long);

    bool access(unsigned long);

    bool setBit(unsigned long, bool);

    unsigned long bits();

    unsigned long ones();

    void updateCounters(long, long);

    TTree *insertBits(long unsigned, long unsigned);

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
