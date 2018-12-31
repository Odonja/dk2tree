//
// Created by anneke on 18/12/18.
//

#ifndef UNTITLED_TTREE_H
#define UNTITLED_TTREE_H

#include <vector>
#include <utility>

typedef std::vector<bool> bit_vector;

// The three main parameters for the TTree and LTree representation
// Values taken from section 6.2.1.
static const unsigned int k = 2;
static const unsigned int B = 512;
static const unsigned int e = 3;

static const unsigned int childCount = 2;

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

    Entry entries[childCount];

    InternalNode() :
            entries{Entry(), Entry()} {}

    ~InternalNode() {
        for (auto &entry : entries) {
            entry.remove();
        }
    }

    unsigned long bits();

    unsigned long ones();
};

/** A leaf node, which consists of a bitvector (represented by vector<bool>) */
struct LeafNode {
    bit_vector bv;

    /// Constructs a leaf with the given number of bits
    explicit LeafNode(unsigned long size) :
            bv(size, false) {}

    /// Constructs a leaf node from the given bit vector
    explicit LeafNode(bit_vector bv) :
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

        explicit Node(bit_vector);
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

    explicit TTree(bit_vector bv) :
            isLeaf(true),
            node(std::move(bv)) {}

    /// The TTree destructor decides which variant of the union to destroy
    ~TTree();

    unsigned long depth();

    unsigned long height();

    Record findChild(unsigned long);

    InternalNode::Entry findLeaf(unsigned long);

    unsigned long rank1(unsigned long);

    bool access(unsigned long);

    bool setBit(unsigned long, bool);

    unsigned long bits();

    unsigned long ones();

    void updateCounters(long, long);

    void split();

    void insertBits(long unsigned, long unsigned);

    void deleteBits(long unsigned, long unsigned);
};

#endif //UNTITLED_TTREE_H
