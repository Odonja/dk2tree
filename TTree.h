//
// Created by anneke on 18/12/18.
//

#ifndef UNTITLED_TTREE_H
#define UNTITLED_TTREE_H

#include <vector>

typedef std::vector<bool> bit_vector;

// The three main parameters for the TTree and LTree representation
// Values taken from section 6.2.1.
static const unsigned int k = 2;
static const unsigned int B = 512;
static const unsigned int e = 3;

static const unsigned int childCount = 2;

struct TTree {
    /// Record type containing the number pf preceding bits and ones, and the
    /// index of a child node in the parent's `entries` list
    struct Record {
        unsigned long b;
        unsigned long o;
        unsigned long i;

        Record(unsigned long b, unsigned long o, unsigned long i):
            b(b), o(o), i(i) {}
    };


    /** TTreeNode is the struct representing a single node (leaf or internal) of the TTree */
    struct TTreeNode;

    /**
     * A struct for the internal nodes of the tree
     * This contains a list of Entries of the form <b, o, P>
     */
    struct InternalNode {
        struct Entry {
            unsigned long b;
            unsigned long o;
            TTreeNode *P;

            Entry():
                    b(0), o(0), P(nullptr)
            {}

            explicit Entry(TTreeNode *P) {
                b = P->bits();
                o = P->ones();
                this->P = P;
            }

            Entry(unsigned long b, unsigned long o, TTreeNode *P):
                    b(b), o(o), P(P)
            {}

            /**
             * This function is called in the destructor of `InternalNode`, to
             * delete the child nodes. It is not part of the destructor of `Entry`,
             * since that causes problems when the struct is used in methods such as
             * `findLeaf`
             */
            void remove() {
                delete P;
            }
        };

        Entry entries[childCount];

        InternalNode():
            entries{Entry(), Entry()}
        {}

        ~InternalNode() {
            for (auto &entry : entries) {
                entry.remove();
            }
        }

        unsigned long bits();
        unsigned long ones();
    };

    /** A leaf node, which consists of a sdsl-bitvector and a rank support structure on that bitvector */
    struct LeafNode {
        bit_vector bv;

        explicit LeafNode(unsigned long size):
            bv(size, false)
        {}

        unsigned long bits();
        unsigned long ones();
    };

    /** A single node is either an internal or leaf node, as indicated by the isLeaf value */
    struct TTreeNode {
        bool isLeaf;

        union Node {
            InternalNode *internalNode;
            LeafNode *leafNode;

            Node() {
                this->leafNode = new LeafNode(B);
            }

            Node(TTreeNode *P1, TTreeNode *P2) {
                this->internalNode = new InternalNode();
                internalNode->entries[0] = InternalNode::Entry(P1);
                internalNode->entries[1] = InternalNode::Entry(P2);
            }

            /// The destructor does nothing, because deletion of the members
            /// has to be handled by `TTreeNode`, which knows which variant it is
            ~Node() { }
        } node;

        TTreeNode():
            isLeaf(true),
            node()
        {}

        TTreeNode(TTreeNode *left, TTreeNode *right):
            isLeaf(false),
            node(left, right)
        {}

        /// The TTreeNode destructor decides which variant of the union to destroy
        ~TTreeNode() {
            if (isLeaf) {
                node.leafNode->~LeafNode();
            } else {
                node.internalNode->~InternalNode();
            }
        }

        Record findChild(unsigned long);
        InternalNode::Entry findLeaf(unsigned long);
        unsigned long rank1(unsigned long);
        bool access(unsigned long);
        bool setBit(unsigned long, bool);
        unsigned long bits();
        unsigned long ones();
    };

    TTreeNode root;
};

#endif //UNTITLED_TTREE_H
