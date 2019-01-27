//
// Created by hugo on 2-1-19.
//

#include "BitVector.h"

/**
 * Gives the value of the n-th bit in the bitvector. This is a read-only operator,
 * since the block_counts must also be updated when writing
 *
 * @param n an index with 0 <= n < bv.size()
 * @return the value of the n-th bit of the bitvector
 */
const bool BitVector::operator[](unsigned long n) {
    return data[n];
}

/**
 * Sets the n-th bit to value b, and returns true if the value changed
 * @param n an index with 0 <= n < size()
 * @param b a boolean
 * @return true iff the previous value of bit n was unequal to b
 */
const bool BitVector::set(unsigned long n, bool b) {
    bool changed = b ^data[n];
    data[n] = b;
    if (changed) {
        unsigned long block = n / BLOCK_SIZE;
        if (b) {
            block_counts[block]++;
        } else {
            block_counts[block]--;
        }
    }
    return changed;
}

/**
 * Performs the rank-operation on this bitvector
 * @param n an index with 0 <= n <= size()
 * @return the number of ones in the bits [0 ... n)
 */
unsigned long BitVector::rank1(unsigned long n) {
    unsigned long end_blocks = n - n % BLOCK_SIZE;
    unsigned long nr_blocks = end_blocks / BLOCK_SIZE;
    unsigned long tot = 0;
    for (unsigned long k = 0; k < nr_blocks; k++) {
        tot += block_counts[k];
    }
    for (unsigned long k = end_blocks; k < n; k++) {
        if (data[k]) {
            tot++;
        }
    }
    return tot;
}

unsigned long BitVector::rangeRank1(unsigned long lo, unsigned long hi) {
    // TODO optimise
    return rank1(hi) - rank1(lo);
}

/**
 * Inserts `size` 0-bits at position `begin`
 * @param begin an index with 0 <= begin <= size()
 * @param size the number of bits to be inserted
 */
void BitVector::insert(unsigned long begin, unsigned long size) {
    data.insert(data.begin() + begin, size, false);
    // If we insert a whole number of blocks, just shift the block_counts
    // data right. Else, we have to recompute this data
    if (begin % BLOCK_SIZE == 0 && size % BLOCK_SIZE == 0) {
        block_counts.insert(
                block_counts.begin() + begin / BLOCK_SIZE,
                size / BLOCK_SIZE,
                0
        );
    } else {
        block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
        recompute(begin);
    }
}

void BitVector::insert(unsigned long begin, const BitVector &from, unsigned long lo, unsigned long hi) {
    data.insert(data.begin() + begin, from.data.begin() + lo, from.data.begin() + hi);
    block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
    recompute(); // TODO: maybe do more efficient update if inserted is whole number of blocks
}

void BitVector::append(const BitVector &from, unsigned long lo, unsigned long hi) {
    data.insert(data.end(), from.data.begin() + lo, from.data.begin() + hi);
    block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
    recompute(); // TODO: maybe do more efficient update if inserted is whole number of blocks
}

/**
 * Deletes the indicated number of bits starting at the indicated index
 * @param begin an index with 0 <= begin < size()
 * @param size the number of bits to be deleted. Should satisfy begin + size <= size()
 */
void BitVector::erase(unsigned long begin, unsigned long size) {
    data.erase(data.begin() + begin, data.begin() + begin + size);
    if (begin % BLOCK_SIZE == 0 && size % BLOCK_SIZE == 0) {
        block_counts.erase(
                block_counts.begin() + begin / BLOCK_SIZE,
                block_counts.begin() + begin / BLOCK_SIZE + size / BLOCK_SIZE
        );
    } else {
        block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
        recompute(begin);
    }
}

/**
 * Private method to re-compute all the values of block_counts from a certain
 * starting point. Used when inserting or deleting bits
 * @param start the first bit that may have changed and require updating the counters
 */
void BitVector::recompute(unsigned long start) {
    for (unsigned long block = start / BLOCK_SIZE; block < block_counts.size(); block++) {
        unsigned long min = block * BLOCK_SIZE;
        unsigned long max = (block + 1) * BLOCK_SIZE;
        if (max > data.size()) {
            max = data.size();
        }
        block_counts[block] = 0;
        for (unsigned long i = min; i < max; i++) {
            if (data[i]) {
                block_counts[block]++;
            }
        }
    }
}
