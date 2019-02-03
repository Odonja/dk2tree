//
// Created by anneke on 18/12/18.
//

#include <iostream>
#include <utility>
#include "TTree.h"

// Constructors and destructors for data types that can't be in TTree.h
TTree::Node::Node() {
    this->internalNode = nullptr;
    this->leafNode = new LeafNode(0);
}

TTree::Node::Node(TTree *P1, TTree *P2) {
    this->leafNode = nullptr;
    this->internalNode = new InternalNode(P1, P2);
}

TTree::Node::Node(BitVector bv) {
    this->internalNode = nullptr;
    this->leafNode = new LeafNode(std::move(bv));
}

TTree::Node::Node(unsigned long size) {
    this->internalNode = nullptr;
    this->leafNode = new LeafNode(size);
}

InternalNode::InternalNode(TTree *left, TTree *right, TTree *parent) :
        size(2),
        entries{Entry(left), Entry(right), Entry()} {
    left->parent = parent;
    left->indexInParent = 0;
    right->parent = parent;
    right->indexInParent = 1;
}

TTree::~TTree() {
    if (isLeaf) {
        delete node.leafNode;
    } else {
        delete node.internalNode;
    }
}

InternalNode::Entry::Entry(TTree *P) :
        b(P->bits()),
        o(P->ones()),
        P(P) {}

void InternalNode::Entry::remove() {
    delete P;
}

Record TTree::findChild(unsigned long n) {
    if (this->isLeaf) {
        return {0, 0, 0};
    } else {
        unsigned long bitsBefore = 0;
        unsigned long onesBefore = 0;

        InternalNode *node = this->node.internalNode;
        unsigned long i;
        for (i = 0; i < node->size; i++) {
            auto &entry = node->entries[i];
            if (bitsBefore + entry.b > n) {
                return {bitsBefore, onesBefore, i};
            }
            bitsBefore += entry.b;
            onesBefore += entry.o;
        }
        // If the required bit is one after the last bit in this tree,
        // return the last child anyway
        // This is necessary for appending bits
        if (i == node->size && bitsBefore == n) {
            auto &entry = node->entries[i - 1];
            return {bitsBefore - entry.b, onesBefore - entry.o, i - 1};
        }
    }
    // If we reach this point, that means that the size of this subtree is less than n
    // so the input parameter was out of range
    // TODO throw exception or something
}

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

unsigned long TTree::rank1(unsigned long n) {
    auto entry = findLeaf(n);
    auto &bv = entry.P->node.leafNode->bv;
    return entry.o + bv.rank1(n - entry.b);
}

bool TTree::access(unsigned long n) {
    auto entry = findLeaf(n);
    return entry.P->node.leafNode->bv[n - entry.b];
}

bool TTree::setBit(unsigned long n, bool b) {
    // Find the leaf node that contains this bit
    auto entry = findLeaf(n);
    BitVector &bv = entry.P->node.leafNode->bv;
    bool changed = bv.set(n - entry.b, b);

    if (changed) {
        // Change the one-counters all the way up from this leaf
        entry.P->updateCounters(0, b ? 1ul : -1ul);
    }
    return changed;
}

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

TTree *TTree::insertBits(long unsigned index, long unsigned count) {
    auto entry = findLeaf(index);
    auto leaf = entry.P;
    auto &bv = leaf->node.leafNode->bv;
    bv.insert(index - entry.b, count);
    leaf->updateCounters(count, 0);

    // Split this node up into two if it exceeds the size limit
    return leaf->checkSizeUpper();
}

TTree *TTree::deleteBits(long unsigned index, long unsigned count) {
    auto entry = findLeaf(index);
    auto leaf = entry.P;
    auto &bv = leaf->node.leafNode->bv;
    long unsigned start = index - entry.b;
    long unsigned end = start + count;
    long unsigned deletedOnes = bv.rangeRank1(start, end);
    bv.erase(start, count);
    leaf->updateCounters(-count, -deletedOnes);
    return leaf->checkSizeLower();
}

unsigned long TTree::depth() {
    if (parent == nullptr) {
        return 0;
    } else {
        return 1 + parent->depth();
    }
}

unsigned long TTree::height() {
    if (isLeaf) {
        return 0;
    } else {
        auto &entries = node.internalNode->entries;
        unsigned long max = 0;
        for (auto &entry : entries) {
            unsigned long depth = entry.P->height();
            if (depth > max) {
                max = depth;
            }
        }
        return max + 1;
    }
}

unsigned long TTree::size() {
    if (isLeaf) {
        return node.leafNode->bits() / block;
    } else {
        return node.internalNode->size;
    }
}

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
    return bv.rank1(bv.size());
}

// All the methods related to maintaining the B+Tree structure when
// inserting/deleting data

InternalNode::Entry InternalNode::popFirst() {
    // Take the first entry out, move everything else left
    InternalNode::Entry result = this->entries[0];
    for (unsigned long i = 1; i < this->size; i++) {
        entries[i - 1] = entries[i];
        entries[i - 1].P->indexInParent = i - 1;
    }
    size--;
    entries[size] = Entry();
    return result;
}

InternalNode::Entry InternalNode::popLast() {
    // Take the last entry out
    InternalNode::Entry result = entries[size - 1];
    size--;
    entries[size] = Entry();
    return result;
}

void InternalNode::insert(unsigned long idx, InternalNode::Entry entry) {
    // Move everything from idx onwards right
    for (unsigned long i = size; i > idx; i--) {
        entries[i] = entries[i - 1];
        entries[i].P->indexInParent = i;
    }
    entries[idx] = entry;
    entries[idx].P->indexInParent = idx;
    size++;
}

void InternalNode::append(InternalNode::Entry entry) {
    entry.P->indexInParent = size;
    entries[size] = entry;
    size++;
}

void InternalNode::remove(unsigned long idx) {
    size--;
    for (unsigned long i = idx; i < size; i++) {
        entries[i] = entries[i + 1];
        entries[i].P->indexInParent = i;
    }
    entries[size] = Entry();
}

TTree *TTree::checkSizeUpper() {
    if (isLeaf && size() > leafSizeMax) {
        if (!trySpillLeaf()) {
            return splitLeaf();
        }
    } else if (!isLeaf && size() > nodeSizeMax) {
        if (!trySpillInternal()) {
            return splitInternal();
        }
    }
    return nullptr;
}

TTree *TTree::checkSizeLower() {
    bool isRoot = (parent == nullptr);
    if (isRoot && (size() >= 2 || isLeaf)) {
        return nullptr;
    } else if (isLeaf && size() < leafSizeMin) {
        if (!tryStealLeaf()) {
            return mergeLeaf();
        }
    } else if (!isLeaf && size() < nodeSizeMin) {
        if (!tryStealInternal()) {
            return mergeInternal();
        }
    } else {
        return nullptr;
    }
}

bool TTree::trySpillInternal() {
    if (parent == nullptr) {
//        printf("cant spill internal node at root\n");
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() < nodeSizeMax) {
//        printf("spilling internal node left\n");
        this->moveLeftInternal();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() < nodeSizeMax) {
//        printf("spilling internal node right\n");
        this->moveRightInternal();
        return true;
    } else {
//        printf("could not spill internal node\n");
        return false;
    }
}

bool TTree::tryStealInternal() {
    if (parent == nullptr) {
//        printf("cant steal internal node at root\n");
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() > nodeSizeMin) {
//        printf("stealing internal node left\n");
        entries[idx - 1].P->moveRightInternal();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() > nodeSizeMin) {
//        printf("stealing internal node right\n");
        entries[idx + 1].P->moveLeftInternal();
        return true;
    } else {
//        printf("could not steal internal node\n");
        return false;
    }
}

bool TTree::trySpillLeaf() {
    if (parent == nullptr) {
//        printf("cant spill leaf at root\n");
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() < leafSizeMax) {
//        printf("spilling leaf left\n");
        this->moveLeftLeaf();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() < leafSizeMax) {
//        printf("spilling leaf right\n");
        this->moveRightLeaf();
        return true;
    } else {
//        printf("could not spill leaf\n");
        return false;
    }
}

bool TTree::tryStealLeaf() {
    if (parent == nullptr) {
//        printf("cant steal leaf at root\n");
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() > leafSizeMin) {
//        printf("stealing leaf from left\n");
        entries[idx - 1].P->moveRightLeaf();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() > leafSizeMin) {
//        printf("stealing leaf from right\n");
        entries[idx + 1].P->moveLeftLeaf();
        return true;
    } else {
//        printf("could not steal leaf\n");
        return false;
    }
}

void TTree::moveLeftInternal() {
//    printf("moving internal node left\n");
    TTree *parent = this->parent;
    unsigned long idx = this->indexInParent;
    TTree *sibling = parent->node.internalNode->entries[idx - 1].P;

    // Move the first child of `this` to the end of the left sibling
    InternalNode::Entry toMove = this->node.internalNode->popFirst();
    unsigned long d_b = toMove.b;
    unsigned long d_o = toMove.o;
    toMove.P->parent = sibling;
    sibling->node.internalNode->append(toMove);

    // Finally, update the parent's b and o counters for `this` and `sibling`
    // The number of bits/ones in `toMove` is subtracted from `this`, but added to `sibling`
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx].o -= d_o;

    parent->node.internalNode->entries[idx - 1].b += d_b;
    parent->node.internalNode->entries[idx - 1].o += d_o;
}

void TTree::moveRightInternal() {
//    printf("moving internal node right\n");
    unsigned long idx = this->indexInParent;
    TTree *sibling = parent->node.internalNode->entries[idx + 1].P;

    // Move the last child of `this` to the start of the left sibling
    InternalNode::Entry toMove = this->node.internalNode->popLast();
    unsigned long d_b = toMove.b;
    unsigned long d_o = toMove.o;
    toMove.P->parent = sibling;
    sibling->node.internalNode->insert(0, toMove);

    // Finally, update the parent's b and o counters for `this` and `sibling`
    // The number of bits/ones in `toMove` is subtracted from `this`, but added to `sibling`
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx].o -= d_o;

    parent->node.internalNode->entries[idx + 1].b += d_b;
    parent->node.internalNode->entries[idx + 1].o += d_o;
}

void TTree::moveLeftLeaf() {
//    printf("moving leaf left\n");
    unsigned long idx = indexInParent;
    TTree *sibling = parent->node.internalNode->entries[idx - 1].P;
    // Take the first k*k block of `this`, and append it to `sibling`
    BitVector &right = node.leafNode->bv;
    BitVector &left = sibling->node.leafNode->bv;
    unsigned long d_b = block;
    unsigned long d_o = right.rank1(block);
    left.append(right, 0, block);
    right.erase(0, block);

    // Update the parent's b and o counters
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx].o -= d_o;

    parent->node.internalNode->entries[idx - 1].b += d_b;
    parent->node.internalNode->entries[idx - 1].o += d_o;
}

void TTree::moveRightLeaf() {
//    printf("moving leaf right\n");
    unsigned long idx = indexInParent;
    TTree *sibling = parent->node.internalNode->entries[idx + 1].P;
    // Take the first k*k block of `this`, and append it to `sibling`
    BitVector &left = node.leafNode->bv;
    BitVector &right = sibling->node.leafNode->bv;
    unsigned long hi = left.size();
    unsigned long lo = hi - block;
    unsigned long d_b = block;
    unsigned long d_o = left.rangeRank1(lo, hi);
    right.insert(0, left, lo, hi);
    left.erase(lo, hi - lo);

    // Update the parent's b and o counters
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx].o -= d_o;

    parent->node.internalNode->entries[idx + 1].b += d_b;
    parent->node.internalNode->entries[idx + 1].o += d_o;
}

TTree *TTree::splitInternal() {
    auto &entries = this->node.internalNode->entries;
    unsigned long n = this->size();
    unsigned long mid = n / 2;
    auto newNode = new TTree();
    newNode->parent = parent;
    newNode->isLeaf = false;
    newNode->node.leafNode = nullptr;
    newNode->node.internalNode = new InternalNode();
    unsigned long d_b = 0, d_o = 0; // Count bits/ones in right half
    for (unsigned long i = mid; i < n; i++) {
        auto entry = entries[i];
        if (entry.P != nullptr) {
            entry.P->parent = newNode;
            entry.P->indexInParent = i - mid;
            d_b += entry.b;
            d_o += entry.o;
        }
        newNode->node.internalNode->entries[i - mid] = entry;
        entries[i] = InternalNode::Entry();
    }
    newNode->node.internalNode->size = n - mid;
    node.internalNode->size = mid;
    if (parent == nullptr) {
//        printf("splitting internal node = root\n");
        auto *newRoot = new TTree(this, newNode);
        return newRoot;
    } else {
//        printf("splitting internal node != root\n");
        parent->node.internalNode->entries[indexInParent].b -= d_b;
        parent->node.internalNode->entries[indexInParent].o -= d_o;
        parent->node.internalNode->insert(indexInParent + 1, InternalNode::Entry(newNode));
        return parent->checkSizeUpper();
    }
}

TTree *TTree::splitLeaf() {
    unsigned long n = this->node.leafNode->bits();
    unsigned long mid = n / 2;
    mid -= mid % block;
    auto &left = this->node.leafNode->bv;
    auto right = BitVector(left, mid, n);
    left.erase(mid, n - mid);
    auto *newNode = new TTree(right);
    if (parent == nullptr) {
//        printf("splitting leaf = root\n");
        auto *newRoot = new TTree(this, newNode);
        return newRoot;
    } else {
//        printf("splitting leaf != root\n");
        unsigned long idx = indexInParent;
        newNode->parent = parent;
        InternalNode::Entry entry(newNode);
        parent->node.internalNode->insert(indexInParent + 1, entry);
        parent->node.internalNode->entries[idx].b -= entry.b;
        parent->node.internalNode->entries[idx].o -= entry.o;
        return parent->checkSizeUpper();
    }
}

TTree *TTree::mergeInternal() {
    // If we are the root and we are too small, then we have only one child
    if (parent == nullptr) {
        // Delete this, our only child should become the root
        TTree *child = node.internalNode->entries[0].P;
        // Overwrite the pointer in the entry, so that it is not deleted
        node.internalNode->entries[0].P = nullptr;
        delete this;
        child->parent = nullptr;
        child->indexInParent = 0;
        return child;
    }

    unsigned long idx = indexInParent;
    TTree *left = nullptr, *right = nullptr;
    if (idx > 0) {
        left = parent->node.internalNode->entries[idx - 1].P;
        right = this;
        idx--;
//        printf("merging internal node left\n");
    } else {
        left = this;
        right = parent->node.internalNode->entries[idx + 1].P;
//        printf("merging internal node right\n");
    }

    // Merge `left` and `right` into one node
    auto &internalNode = left->node.internalNode;
    unsigned long n = right->size();
    unsigned long d_b = 0, d_o = 0;
    for (unsigned i = 0; i < n; i++) {
        auto entry = right->node.internalNode->entries[i];
        right->node.internalNode->entries[i].P = nullptr;
        d_b += entry.b;
        d_o += entry.o;
        entry.P->parent = right;
        internalNode->append(entry);
    }
    // Delete the right child, and update the b and o counters for left
    parent->node.internalNode->remove(idx + 1);
    parent->node.internalNode->entries[idx].b += d_b;
    parent->node.internalNode->entries[idx].o += d_o;
    return parent->checkSizeLower();
}

TTree *TTree::mergeLeaf() {
    if (parent == nullptr) {
        return nullptr;
    }
    unsigned long idx = indexInParent;
    TTree *left = nullptr, *right = nullptr;
    if (idx > 0) {
        left = parent->node.internalNode->entries[idx].P;
        right= this;
        idx--;
//        printf("merging leaf left\n");
    } else {
        left = this;
        right = parent->node.internalNode->entries[idx + 1].P;
//        printf("merging leaf right\n");
    }
    auto &leftBits = left->node.leafNode->bv;
    auto &rightBits = right->node.leafNode->bv;
    // Append `right`s bits to `left`
    leftBits.append(rightBits, 0, rightBits.size());
    // Update the b and o for `left`, and delete `right`
    unsigned long d_b = parent->node.internalNode->entries[idx + 1].b;
    unsigned long d_o = parent->node.internalNode->entries[idx + 1].o;
    parent->node.internalNode->entries[idx].b += d_b;
    parent->node.internalNode->entries[idx].o += d_o;
    parent->node.internalNode->remove(idx + 1);
    return parent->checkSizeLower();
}
