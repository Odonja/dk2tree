//
// Created by anneke on 18/12/18.
//

#include <iostream>
#include <utility>
#include "LTree.h"

// Constructors and destructors for data types that can't be in LTree.h
LTree::Node::Node() {
    this->internalNode = nullptr;
    this->leafNode = new LLeafNode(0);
}

LTree::Node::Node(LTree *P1, LTree *P2) {
    this->leafNode = nullptr;
    this->internalNode = new LInternalNode(P1, P2);
}

LTree::Node::Node(BitVector bv) {
    this->internalNode = nullptr;
    this->leafNode = new LLeafNode(std::move(bv));
}

LTree::Node::Node(unsigned long size) {
    this->internalNode = nullptr;
    this->leafNode = new LLeafNode(size);
}

LInternalNode::LInternalNode(LTree *left, LTree *right, LTree *parent) :
        size(2),
        entries{Entry(left), Entry(right), Entry()} {
    left->parent = parent;
    left->indexInParent = 0;
    right->parent = parent;
    right->indexInParent = 1;
}

LTree::~LTree() {
    if (isLeaf) {
        delete node.leafNode;
    } else {
        delete node.internalNode;
    }
}

LInternalNode::Entry::Entry(LTree *P) :
        b(P->bits()),
        P(P) {}

void LInternalNode::Entry::remove() {
    delete P;
}

LRecord LTree::findChild(unsigned long n) {
    unsigned long bitsBefore = 0;

    LInternalNode *node = this->node.internalNode;
    unsigned long i;
    for (i = 0; i < node->size; i++) {
        auto &entry = node->entries[i];
        if (bitsBefore + entry.b > n) {
            return {bitsBefore, i};
        }
        bitsBefore += entry.b;
    }
    // If the required bit is one after the last bit in this tree,
    // return the last child anyway
    // This is necessary for appending bits
    if (i == node->size && bitsBefore == n) {
        auto &entry = node->entries[i - 1];
        return {bitsBefore - entry.b, i - 1};
    }
    // If we reach this point, that means that the size of this subtree is less than n
    // so the input parameter was out of range
    throw std::range_error("LTree: index out of range");
}

LInternalNode::Entry LTree::findLeaf(unsigned long n, vector<LNesbo> *path) {
    if (path == nullptr) {
        auto *current = this;
        unsigned long bitsBefore = 0;
        while (!current->isLeaf) {
            auto record = current->findChild(n - bitsBefore);
            bitsBefore += record.b;
            current = current->node.internalNode->entries[record.i].P;
        }
        return {bitsBefore, current};
    } else {
        return findLeaf2(n, *path);
    }
}

LInternalNode::Entry LTree::findLeaf2(unsigned long n, vector<LNesbo> &path) {
    LTree *current = nullptr;
    unsigned long bitsBefore = 0;
    if (path.empty()) {
        // If the path is empty, we have to do a regular findLeaf and store the
        // results in the path vector
        current = this;
        bitsBefore = 0;
    } else {
        // Start at the end of the path (which is a leaf), and move up until we
        // reach the subtree that also contains the desired bit
        unsigned long k = path.size() - 1;
        LNesbo nesbo = path[k];
        // While the subtree nested at `nesbo.node` is entirely before, or
        // entirely after the bit we are looking for...
        while (n < nesbo.bitsBefore || nesbo.bitsBefore + nesbo.size <= n) {
            // Go `up` one level if possible. Else, we are at the root so do a
            // regular search from there
            path.pop_back();
            if (k == 0) {
                current = nesbo.node;
                break;
            }
            k -= 1;
            nesbo = path[k];
        };

        // If we didn't exit early, then set the start of the search path to the
        // last entry of path that still exists
        if (current == nullptr) {
            current = nesbo.node->node.internalNode->entries[nesbo.index].P;
            bitsBefore = nesbo.bitsBefore;
        }
    }
    // Now, we do a regular search from `current` and add the intermediate
    // nodes we visit to `path`
    while (!current->isLeaf) {
        auto record = current->findChild(n - bitsBefore);
        bitsBefore += record.b;
        auto next = current->node.internalNode->entries[record.i];
        path.emplace_back(current, record.i, next.b, bitsBefore);
        current = next.P;
    }

    return {bitsBefore, current};
}

bool LTree::access(unsigned long n, vector<LNesbo> *path) {
    auto entry = findLeaf(n, path);
    return entry.P->node.leafNode->bv[n - entry.b];
}

bool LTree::setBit(unsigned long n, bool b, vector<LNesbo> *path) {
    // Find the leaf node that contains this bit
    auto entry = findLeaf(n, path);
    BitVector &bv = entry.P->node.leafNode->bv;
    bool changed = bv.set(n - entry.b, b);

    return changed;
}

void LTree::updateCounters(long dBits) {
    LTree *current = this;
    // Go up in the tree until we reach the root
    while (current->parent != nullptr) {
        // Take the entry in `current`s parent that points to `current`,
        // and update its `b` counter.
        auto parent = current->parent;
        auto &entry = parent->node.internalNode->entries[current->indexInParent];
        entry.b += dBits;

        current = parent;
    }
}

LTree *LTree::insertBits(long unsigned index, long unsigned count,
                         vector<LNesbo> *path) {
    auto entry = findLeaf(index, path);
    auto leaf = entry.P;
    auto &bv = leaf->node.leafNode->bv;
    bv.insert(index - entry.b, count);
    leaf->updateCounters(count);

    // Split this node up into two if it exceeds the size limit
    return leaf->checkSizeUpper();
}

LTree *LTree::deleteBits(long unsigned index, long unsigned count,
                         vector<LNesbo> *path) {
    auto entry = findLeaf(index, path);
    auto leaf = entry.P;
    auto &bv = leaf->node.leafNode->bv;
    long unsigned start = index - entry.b;
    long unsigned end = start + count;
    bv.erase(start, end);
    leaf->updateCounters(-count);
    return leaf->checkSizeLower();
}

LTree *LTree::insertBlock(long unsigned index, vector<LNesbo> *path) {
    return this->insertBits(index, block, path);
}

LTree *LTree::deleteBlock(long unsigned index, vector<LNesbo> *path) {
    return this->deleteBits(index, block, path);
}

unsigned long LTree::depth() {
    if (parent == nullptr) {
        return 0;
    } else {
        return 1 + parent->depth();
    }
}

unsigned long LTree::height() {
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

unsigned long LTree::size() {
    if (isLeaf) {
        return node.leafNode->bits() / block;
    } else {
        return node.internalNode->size;
    }
}

unsigned long LTree::bits() {
    if (isLeaf) {
        return node.leafNode->bits();
    } else {
        return node.internalNode->bits();
    }
}

unsigned long LInternalNode::bits() {
    unsigned long total = 0;
    for (const auto &entry : entries) {
        total += entry.b;
    }
    return total;
}

unsigned long LLeafNode::bits() {
    return bv.size();
}

// All the methods related to maintaining the B+Tree structure when
// inserting/deleting data

LInternalNode::Entry LInternalNode::popFirst() {
    // Take the first entry out, move everything else left
    LInternalNode::Entry result = this->entries[0];
    this->remove(0);
    return result;
}

LInternalNode::Entry LInternalNode::popLast() {
    // Take the last entry out
    size--;
    LInternalNode::Entry result = entries[size];
    entries[size] = Entry();
    return result;
}

void LInternalNode::insert(unsigned long idx, LInternalNode::Entry entry) {
    // Move everything from idx onwards right
    for (unsigned long i = size; i > idx; i--) {
        entries[i] = entries[i - 1];
        entries[i].P->indexInParent = i;
    }
    entries[idx] = entry;
    entries[idx].P->indexInParent = idx;
    size++;
}

void LInternalNode::append(LInternalNode::Entry entry) {
    entry.P->indexInParent = size;
    entries[size] = entry;
    size++;
}

void LInternalNode::remove(unsigned long idx) {
    size--;
    for (unsigned long i = idx; i < size; i++) {
        entries[i] = entries[i + 1];
        entries[i].P->indexInParent = i;
    }
    entries[size] = Entry();
}

LTree *LTree::checkSizeUpper() {
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

LTree *LTree::checkSizeLower() {
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
    }
    return nullptr;
}

bool LTree::trySpillInternal() {
    if (parent == nullptr) {
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() < nodeSizeMax) {
        this->moveLeftInternal();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() < nodeSizeMax) {
        this->moveRightInternal();
        return true;
    } else {
        return false;
    }
}

bool LTree::tryStealInternal() {
    if (parent == nullptr) {
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() > nodeSizeMin) {
        entries[idx - 1].P->moveRightInternal();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() > nodeSizeMin) {
        entries[idx + 1].P->moveLeftInternal();
        return true;
    } else {
        return false;
    }
}

bool LTree::trySpillLeaf() {
    if (parent == nullptr) {
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() < leafSizeMax) {
        this->moveLeftLeaf();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() < leafSizeMax) {
        this->moveRightLeaf();
        return true;
    } else {
        return false;
    }
}

bool LTree::tryStealLeaf() {
    if (parent == nullptr) {
        return false;
    }
    unsigned long idx = indexInParent;
    unsigned long n = parent->size();
    auto &entries = parent->node.internalNode->entries;
    if (idx > 0 && entries[idx - 1].P->size() > leafSizeMin) {
        entries[idx - 1].P->moveRightLeaf();
        return true;
    } else if (idx + 1 < n && entries[idx + 1].P->size() > leafSizeMin) {
        entries[idx + 1].P->moveLeftLeaf();
        return true;
    } else {
        return false;
    }
}

void LTree::moveLeftInternal() {
    LTree *parent = this->parent;
    unsigned long idx = this->indexInParent;
    LTree *sibling = parent->node.internalNode->entries[idx - 1].P;

    // Move the first child of `this` to the end of the left sibling
    LInternalNode::Entry toMove = this->node.internalNode->popFirst();
    unsigned long d_b = toMove.b;
    toMove.P->parent = sibling;
    sibling->node.internalNode->append(toMove);

    // Finally, update the parent's b counter for `this` and `sibling`
    // The number of bits in `toMove` is subtracted from `this`, but added to `sibling`
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx - 1].b += d_b;
}

void LTree::moveRightInternal() {
    unsigned long idx = this->indexInParent;
    LTree *sibling = parent->node.internalNode->entries[idx + 1].P;

    // Move the last child of `this` to the start of the left sibling
    LInternalNode::Entry toMove = this->node.internalNode->popLast();
    unsigned long d_b = toMove.b;
    toMove.P->parent = sibling;
    sibling->node.internalNode->insert(0, toMove);

    // Finally, update the parent's b counter for `this` and `sibling`
    // The number of bits in `toMove` is subtracted from `this`, but added to `sibling`
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx + 1].b += d_b;
}

void LTree::moveLeftLeaf() {
    unsigned long idx = indexInParent;
    LTree *sibling = parent->node.internalNode->entries[idx - 1].P;
    // Take the first k*k block of `this`, and append it to `sibling`
    BitVector &right = node.leafNode->bv;
    BitVector &left = sibling->node.leafNode->bv;
    unsigned long d_b = block;
    left.append(right, 0, block);
    right.erase(0, block);

    // Update the parent's b counter
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx - 1].b += d_b;
}

void LTree::moveRightLeaf() {
    unsigned long idx = indexInParent;
    LTree *sibling = parent->node.internalNode->entries[idx + 1].P;
    // Take the first k*k block of `this`, and append it to `sibling`
    BitVector &left = node.leafNode->bv;
    BitVector &right = sibling->node.leafNode->bv;
    unsigned long hi = left.size();
    unsigned long lo = hi - block;
    unsigned long d_b = block;
    right.insert(0, left, lo, hi);
    left.erase(lo, hi);

    // Update the parent's b counter
    parent->node.internalNode->entries[idx].b -= d_b;
    parent->node.internalNode->entries[idx + 1].b += d_b;
}

LTree *LTree::splitInternal() {
    auto &entries = this->node.internalNode->entries;
    unsigned long n = this->size();
    unsigned long mid = n / 2;
    auto newNode = new LTree();
    newNode->parent = parent;
    newNode->isLeaf = false;
    newNode->node.leafNode = nullptr;
    newNode->node.internalNode = new LInternalNode();
    unsigned long d_b = 0; // Count bits in right half
    for (unsigned long i = mid; i < n; i++) {
        auto entry = entries[i];
        if (entry.P != nullptr) {
            entry.P->parent = newNode;
            entry.P->indexInParent = i - mid;
            d_b += entry.b;
        }
        newNode->node.internalNode->entries[i - mid] = entry;
        entries[i] = LInternalNode::Entry();
    }
    newNode->node.internalNode->size = n - mid;
    node.internalNode->size = mid;
    if (parent == nullptr) {
        auto *newRoot = new LTree(this, newNode);
        return newRoot;
    } else {
        parent->node.internalNode->entries[indexInParent].b -= d_b;
        parent->node.internalNode->insert(indexInParent + 1,
                                          {d_b, newNode});
        return parent->checkSizeUpper();
    }
}

LTree *LTree::splitLeaf() {
    unsigned long n = this->node.leafNode->bits();
    unsigned long mid = n / 2;
    mid -= mid % block;
    auto &left = this->node.leafNode->bv;
    auto right = BitVector(left, mid, n);
    left.erase(mid, n);
    auto *newNode = new LTree(right);
    if (parent == nullptr) {
        auto *newRoot = new LTree(this, newNode);
        return newRoot;
    } else {
        unsigned long idx = indexInParent;
        newNode->parent = parent;
        LInternalNode::Entry entry(newNode);
        parent->node.internalNode->insert(indexInParent + 1, entry);
        parent->node.internalNode->entries[idx].b -= entry.b;
        return parent->checkSizeUpper();
    }
}

LTree *LTree::mergeInternal() {
    // If we are the root and we are too small, then we have only one child
    if (parent == nullptr) {
        // Delete this, our only child should become the root
        LTree *child = node.internalNode->entries[0].P;
        // Overwrite the pointer in the entry, so that it is not deleted
        node.internalNode->entries[0].P = nullptr;
        delete this;
        child->parent = nullptr;
        child->indexInParent = 0;
        return child;
    }

    unsigned long idx = indexInParent;
    LTree *left = nullptr, *right = nullptr;
    if (idx > 0) {
        left = parent->node.internalNode->entries[idx - 1].P;
        right = this;
        idx--;
    } else {
        left = this;
        right = parent->node.internalNode->entries[idx + 1].P;
    }

    // Merge `left` and `right` into one node
    auto &internalNode = left->node.internalNode;
    unsigned long n = right->size();
    unsigned long d_b = 0;
    for (unsigned i = 0; i < n; i++) {
        auto entry = right->node.internalNode->entries[i];
        right->node.internalNode->entries[i].P = nullptr;
        d_b += entry.b;
        entry.P->parent = left;
        internalNode->append(entry);
    }
    // Delete the right child, and update the b counter for left
    parent->node.internalNode->remove(idx + 1);
    parent->node.internalNode->entries[idx].b += d_b;
    return parent->checkSizeLower();
}

LTree *LTree::mergeLeaf() {
    if (parent == nullptr) {
        return nullptr;
    }
    unsigned long idx = indexInParent;
    LTree *left = nullptr, *right = nullptr;
    if (idx > 0) {
        left = parent->node.internalNode->entries[idx - 1].P;
        right = this;
        idx--;
    } else {
        left = this;
        right = parent->node.internalNode->entries[idx + 1].P;
    }
    auto &leftBits = left->node.leafNode->bv;
    auto &rightBits = right->node.leafNode->bv;
    // Append `right`s bits to `left`
    leftBits.append(rightBits, 0, rightBits.size());
    // Update the b for `left`, and delete `right`
    unsigned long d_b = parent->node.internalNode->entries[idx + 1].b;
    parent->node.internalNode->entries[idx].b += d_b;
    parent->node.internalNode->remove(idx + 1);
    return parent->checkSizeLower();
}

unsigned long LTree::memoryUsage() {
    unsigned long result = sizeof(LTree);
    if (isLeaf) {
        result += node.leafNode->bv.memoryUsage();
    } else {
        result += sizeof(LInternalNode);
        auto &entries = node.internalNode->entries;
        for (auto &entry : entries) {
            if (entry.P != nullptr) {
                result += entry.P->memoryUsage();
            }
        }
    }
    return result;
}
