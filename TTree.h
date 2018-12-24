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

class TTree {
    /** TTreeNode is the struct representing a single node (leaf or internal) of the TTree */
    struct TTreeNode;

    /**
     * A struct for the internal nodes of the tree
     * This contains a list of Entries of the form <b, o, P>
     */
    struct InternalNode {
        struct Entry {
            unsigned int b;
            unsigned int o;
            TTreeNode *P;

            Entry():
                    b(0), o(0), P(nullptr)
            {}

            Entry(unsigned int b, unsigned int o, TTreeNode *P):
                    b(b), o(o), P(P)
            {}
        };

        Entry entries[2];
    };

    /** A leaf node, which consists of a sdsl-bitvector and a rank support structure on that bitvector */
    struct LeafNode {
        bit_vector bv;
        rank_support_v<> rs;
    };

    /** A single node is either an internal or leaf node, as indicated by the isLeaf value */
    struct TTreeNode {
        bool isLeaf;

        union {
            InternalNode internalNode;
            LeafNode leafNode;
        } node;
    };
};

#endif //UNTITLED_TTREE_H
