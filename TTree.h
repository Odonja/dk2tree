//
// Created by anneke on 18/12/18.
//

#ifndef UNTITLED_TTREE_H
#define UNTITLED_TTREE_H

#include <sdsl/bit_vectors.hpp>
#include <sdsl/rank_support_v.hpp>

using namespace sdsl;

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

            Entry(unsigned long b, unsigned long o, TTreeNode *P):
                    b(b), o(o), P(P)
            {}

            ~Entry() {
            }

            void remove() {
                std::cout << "Weg ermee" << std::endl;
                delete P;
            }
        };

        Entry entries[2];

        InternalNode():
            entries{Entry(), Entry()}
        {}

        ~InternalNode() {
            for (auto &entry : entries) {
                entry.remove();
            }
        }
    };

    /** A leaf node, which consists of a sdsl-bitvector and a rank support structure on that bitvector */
    struct LeafNode {
        bit_vector bv;
        rank_support_v<> rs;

        explicit LeafNode(unsigned long size):
            bv(size, 0),
            rs(&bv)
        {}
    };

    /** A single node is either an internal or leaf node, as indicated by the isLeaf value */
    struct TTreeNode {
        bool isLeaf;

        union Node {
            InternalNode internalNode;
            LeafNode leafNode;

            Node() {
                new(&leafNode) LeafNode(B);
            }

            /// The constructor does nothing, because deletion of the members
            /// has to be handled by `TTreeNode`, which knows which variant it is
            ~Node() { }
        } node;

        Record findChild(unsigned long);
        InternalNode::Entry findLeaf(unsigned long);
        unsigned long rank1(unsigned long);
        bool access(unsigned long);
        bool setBit(unsigned long, bool);

        TTreeNode():
            isLeaf(true),
            node()
        {}

        /// The TTreeNode destructor decides which variant of the union to destroy
        ~TTreeNode() {
            if (isLeaf) {
                node.leafNode.~LeafNode();
            } else {
                node.internalNode.~InternalNode();
            }
        }
    };

    TTreeNode root;
};

#endif //UNTITLED_TTREE_H
