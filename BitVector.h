//
// Created by hugo on 2-1-19.
//

#ifndef UNTITLED_BITVECTOR_H
#define UNTITLED_BITVECTOR_H

#include <vector>
#include <cstdint>

using std::vector;

/**
 * The number of bits in one block. This number should be < 256, so that the
 * counts can be stored in a single 8-bit number.
 */
static const unsigned long BLOCK_SIZE = 32;

/**
 * A simple bitvector containing the `raw` bits in a vector<bool>, as well as
 * a list of the number of ones in each block, to speed up rank operations
 */
struct BitVector {
    vector<bool> data;
    vector<uint8_t> block_counts;

    const bool operator[](unsigned long n);

    const bool set(unsigned long n, bool b);

    unsigned long rank1(unsigned long n);

    void insert(unsigned long begin, unsigned long size);

    void erase(unsigned long begin, unsigned long size);

    unsigned long size() {
        return data.size();
    }

    explicit BitVector(unsigned long size) :
            data(size, false),
            block_counts((size + BLOCK_SIZE - 1) / BLOCK_SIZE, 0) {}

private:
    void recompute(unsigned long start = 0);
};


#endif //UNTITLED_BITVECTOR_H
