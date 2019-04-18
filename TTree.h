//
// Created by anneke on 18/12/18.
//

#ifndef DK2TREE_TTREE_H
#define DK2TREE_TTREE_H

#include "BitVector.h"
#include <utility>
#include "parameters.cpp"

/// Record type containing the number pf preceding bits and ones, and the
/// index of a child node in the parent's `entries` list
struct Record {
    unsigned long b;
    unsigned long o;
    unsigned long i;
};

/// The three main structs forming the tree
/// `TTree` represents one node in the tree, which contains a pointer to its
/// parent (or nullptr for the root) and the index in the parent,
/// as well as either an InternalNode containing entries (b, o, P) or a LeafNode
/// containing a BitVector
struct InternalNode;
struct LeafNode;
struct TTree;

struct Nesbo {
public:
    TTree *node;
    unsigned long index;
    unsigned long size;
    unsigned long bitsBefore;
    unsigned long onesBefore;

    Nesbo(TTree *node, unsigned long index, unsigned long size,
          unsigned long bitsBefore, unsigned long onesBefore) :
            node(node),
            index(index),
            size(size),
            bitsBefore(bitsBefore),
            onesBefore(onesBefore) {}
};

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

        explicit Entry(TTree *);

        /// Construct Entry from the three fields
        /// This method is not unused, but is used as an initialiser list
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

    /// The default constructor creates an empty internal node
    InternalNode() :
            size(0),
            entries{Entry()} {}

    /**
     * Creates a new internal node with the given two children
     *
     * @param left the first child of this node
     * @param right the second child of this node
     * @param parent the parent node, which has this as its internal node
     *        the left and right TTrees have their parent and indexInParent
     *        set correctly as well
     */
    InternalNode(TTree *left, TTree *right, TTree *parent = nullptr);

    /**
     * When an internal node is dropped, clear the entries it points to
     */
    ~InternalNode() {
        for (auto &entry : entries) {
            entry.remove();
        }
    }

    /**
     * Returns the total number of bits in this tree, by summing up the
     * b-parts of the entries
     */
    unsigned long bits();

    /**
     * Returns the total number of ones in this tree, by summing up the
     * o-parts of the entries
     */
    unsigned long ones();

    /**
     * Takes the leftmost entry out of this node and returns it
     * @return
     */
    Entry popFirst();

    /**
     * Takes the rightmost entry out of this node and returns it
     * @return
     */
    Entry popLast();

    /**
     * Adds the given entry to this node at the specified position
     */
    void insert(unsigned long, Entry);

    /**
     * Adds the given entry to the end of this node
     */
    void append(Entry);

    /**
     * Removes the entry at the specified position from this node
     */
    void remove(unsigned long);
};

/** A leaf node, which consists of a bitvector (represented by vector<bool>) */
struct LeafNode {
    BitVector<> bv;

    /**
     * Constructs a leaf with the given number of bits
     */
    explicit LeafNode(unsigned long size) :
            bv(size) {}

    /**
     * Constructs a leaf node from the given bit vector
     * @param bv the bitvector to be moved into this leaf node
     */
    explicit LeafNode(BitVector<> bv) :
            bv(bv) {}

    /**
     * Get the total number of bits stored in this leaf node
     * @return the size in bits of this leaf
     */
    unsigned long bits();

    /**
     * Get the number of ones in this leaf node
     * @return the number of bits in this leaf that are set to 1
     */
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

        explicit Node(BitVector<>);

        explicit Node(unsigned long);
    } node;

    /**
     * Constructs an empty leaf node
     */
    TTree() :
            isLeaf(true),
            node() {}

    /**
     * Constructs a node with the two given `TTree`s as children
     * @param left the first child of this node
     * @param right the second child of this node
     */
    TTree(TTree *left, TTree *right) :
            isLeaf(false),
            node(left, right) {
        left->parent = this;
        left->indexInParent = 0;
        right->parent = this;
        right->indexInParent = 1;
    }

    /**
     * Constructs a leaf node with the given bit vector
     * @param bv the bit vector to be moved into this leaf node
     */
    explicit TTree(BitVector<> bv) :
            isLeaf(true),
            node(bv) {}

    /**
     * Constructs an all-zeros leaf node with the specified size
     * @param size the size of this leaf node in bits
     */
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

    /**
     * Gets the size of this node, either as the number of children (internal
     * node) or in number of k^2 blocks (leaf node)
     * @return the size of this node in number of children or bit blocks,
     *         depending on the type of the node
     */
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
    InternalNode::Entry findLeaf(unsigned long, vector<Nesbo> *path = nullptr);

    InternalNode::Entry findLeaf2(unsigned long, vector<Nesbo> &);

    /**
     * Performs the `rank` operation on the bitvector represented by this tree
     * @param n  an integer with 0 <= n < (numbers of bits in the tree)
     * @return the number of 1-bits in the tree up to position n
     */
    unsigned long rank1(unsigned long, vector<Nesbo> *path = nullptr);

    /**
     * Performs the `access` operation on this subtree
     * @param n the index of a bit in the `TTree`
     * @return the value of the `n`-th bit in the tree
     */
    bool access(unsigned long, vector<Nesbo> *path = nullptr);

    /**
     * Sets the bit at position n to the value of b
     * @param n the index of the bit to set
     * @param b the value to set the bit to
     * @return true if this bit changed, e.g.
     *      if the previous value of the bit was unequal to b
     */
    bool setBit(unsigned long, bool, vector<Nesbo> *path = nullptr);

    /**
     * Gets the total number of bits covered by this subtree
     * @return the total number of bits in this node's subtree
     */
    unsigned long bits();

    /**
     * Gets the total number of ones covered by this subtree
     * @return the total number of 1-bits in this node's subtree
     */
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
     * Inserts one block of k^2 bits at the specified position
     *
     * @return the new root if the tree's root changed, nullptr otherwise
     */
    TTree *insertBlock(long unsigned, vector<Nesbo> *path = nullptr);

    /**
     * Deletes a block of k^2 bits at the specified position
     *
     * @return the new root if the tree's root changed, nullptr otherwise
     */
    TTree *deleteBlock(long unsigned, vector<Nesbo> *path = nullptr);

    unsigned long memoryUsage();

private:
    /**
     * Inserts the given number of bits (set to zero) at the given position in the tree
     *
     * @param index the position at which to insert bits
     * @param count the number of bits to insert
     *
     * @return nullptr in most cases, but a pointer to the new root node
     * if this insert operation created a new root (e.g. when the height of the
     * tree increases)
     *
     * This method assumes that after the insertion, the size of the relevant
     * leaf vector is at most one block over the maximum, which can only be
     * guaranteed when only a single block is inserted. That is why this method
     * is private, and insertBlock is public.
     */
    TTree *
    insertBits(long unsigned, long unsigned, vector<Nesbo> *path = nullptr);

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
     *
     * This method assumes that after the deletion, the size of the relevant
     * leaf vector is at most one block under the minimum, which can only be
     * guaranteed when only a single block is deleted. That is why this method
     * is private, and deleteBlock is public.
     */
    TTree *
    deleteBits(long unsigned, long unsigned, vector<Nesbo> *path = nullptr);

    /**
     * Checks if this node satisfies the maximum size for an internal node
     * or leaf node. If not, tries to spill a node to a sibling, or if that
     * fails will split this node into two and recursively check the parent
     *
     * @return nullptr in most cases, but returns the new root if it has
     *         changed, e.g. if the height of the tree has increased
     */
    TTree *checkSizeUpper();

    /**
     * Checks if this node satisfies the minimum size for an internal node
     * or leaf node. If not, tries to steal a node from a sibling, or if that
     * fails will merge this node with one of the siblings, and recursively
     * check the parent
     *
     * @return nullptr in most cases, but returns the new root if it has
     *         changed, e.g. if the height of the tree has decreased
     */
    TTree *checkSizeLower();

    /**
     * Tries to move a child of an internal node to a sibling, and returns
     * whether it succeeded
     *
     * @return true if a spill could be done, false if not (e.g. if both of
     *         the nodes siblings are of maximum size)
     */
    bool trySpillInternal();

    /**
     * Tries to move a child of an leaf node to a sibling, and returns
     * whether it succeeded
     *
     * @return true if a spill could be done, false if not (e.g. if both of
     *         the nodes siblings are of maximum size)
     */
    bool trySpillLeaf();

    /**
     * Splits this node into two nodes of minimum size, and recursively
     * checks the rest of the tree for meeting size requirements
     *
     * @return nullptr in most cases, but returns the new root if this operation
     *         causes the tree's height to increase, which changes the root
     */
    TTree *splitInternal();

    /**
     * Splits this node into two nodes of minimum size, and recursively
     * checks the rest of the tree for meeting size requirements
     *
     * @return nullptr in most cases, but returns the new root if this operation
     *         causes the tree's height to increase, which changes the root
     */
    TTree *splitLeaf();

    /**
     * Tries to steal a node from one of this node's siblings, and returns true
     * if it succeeds
     *
     * @return true if this operation could be done, false otherwise (e.g. if both
     *         of this node's siblings are of minimum size)
     */
    bool tryStealInternal();

    /**
     * Tries to steal a node from one of this node's siblings, and returns true
     * if it succeeds
     *
     * @return true if this operation could be done, false otherwise (e.g. if both
     *         of this node's siblings are of minimum size)
     */
    bool tryStealLeaf();

    /**
     * Merges this node with a sibling, and recursively checks the rest of the
     * tree for meeting size constraints
     *
     * @return nullptr usually, but returns the new root it it changed due to this
     *         operation, e.g. when the height of the tree changed
     */
    TTree *mergeInternal();

    /**
     * Merges this node with a sibling, and recursively checks the rest of the
     * tree for meeting size constraints
     *
     * @return nullptr usually, but returns the new root it it changed due to this
     *         operation, e.g. when the height of the tree changed
     */
    TTree *mergeLeaf();

    /**
     * Moves a single child (the leftmost child) of this node to the end of the
     * node's left sibling
     */
    void moveLeftInternal();

    /**
     * Moves the rightmost child of this node to the start of the right sibling
     */
    void moveRightInternal();

    /**
     * Moves the leftmost block of k^2 bits to the end of the left sibling
     */
    void moveLeftLeaf();

    /**
     * Moves the rightmost block of k^2 bits to the start of the right sibling
     */
    void moveRightLeaf();
};

#endif //DK2TREE_TTREE_H
