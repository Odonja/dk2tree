//
// Created by hugo on 2-1-19.
//

#ifndef DK2TREE_BITVECTOR_H
#define DK2TREE_BITVECTOR_H

#include <vector>
#include <cstdint>
#include "parameters.cpp"

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
template <unsigned long LENGTH = (B + 63) / 64>
struct BitVector {
    vector<bool> data;
    vector<uint8_t> block_counts;

    /**
     * Gives the value of the n-th bit in the bitvector. This is a read-only operator,
     * since the block_counts must also be updated when writing
     *
     * @param n an index with 0 <= n < bv.size()
     * @return the value of the n-th bit of the bitvector
     */
    const bool operator[](unsigned long n) {
        return data[n];
    }

    /**
     * Sets the n-th bit to value b, and returns true if the value changed
     * @param n an index with 0 <= n < size()
     * @param b a boolean
     * @return true iff the previous value of bit n was unequal to b
     */
    const bool set(unsigned long n, bool b) {
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
    unsigned long rank1(unsigned long n) {
        unsigned long end_blocks = n - n % BLOCK_SIZE;
        unsigned long nr_blocks = end_blocks / BLOCK_SIZE;
        return countBlocks(0, nr_blocks) + countOnesRaw(end_blocks, n);
    }

    /**
     * Returns the number of 1-bits in the interval [lo, hi), which is equal to
     * rank1(hi) - rank1(lo)
     */
    unsigned long rangeRank1(unsigned long lo, unsigned long hi) {
        unsigned long blockLo = (lo + BLOCK_SIZE - 1) / BLOCK_SIZE, blockHi = hi / BLOCK_SIZE;
        if (blockLo > blockHi) {
            return countOnesRaw(lo, hi);
        }
        unsigned long blockStart = blockLo * BLOCK_SIZE, blockEnd = blockHi * BLOCK_SIZE;
        return countOnesRaw(lo, blockStart) + countBlocks(blockLo, blockHi) + countOnesRaw(blockEnd, hi);
    }

    /**
     * Inserts `size` 0-bits at position `begin`
     * @param begin an index with 0 <= begin <= size()
     * @param size the number of bits to be inserted
     */
    void insert(unsigned long begin, unsigned long size) {
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

    /**
     * Inserts the range [lo, hi) of the bit vector `from` into this bit vector
     *
     * @param begin the position in this bit vector to insert into
     * @param from the bit vector to insert a range from
     * @param lo the start of the range in `from` to insert
     * @param hi the end of the range in `from` to insert
     */
    void insert(unsigned long begin, const BitVector &from, unsigned long lo, unsigned long hi) {
        data.insert(data.begin() + begin, from.data.begin() + lo, from.data.begin() + hi);
        block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
        recompute(); // TODO: maybe do more efficient update if inserted is whole number of blocks
    }

    /**
     * Appends the range [lo, hi) of the bit vector `from` into this bit vector
     *
     * @param from the bit vector to append bits from
     * @param lo the start of the range of bits to append
     * @param hi the end of the range of bits to append
     */
    void append(const BitVector &from, unsigned long lo, unsigned long hi) {
        data.insert(data.end(), from.data.begin() + lo, from.data.begin() + hi);
        block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
        recompute(); // TODO: maybe do more efficient update if inserted is whole number of blocks
    }


    /**
     * Deletes the indicated range of bits starting at the indicated index
     * @param lo an index with 0 <= lo <= size()
     * @param hi the end of the range of bits to be deleted. Should satisfy lo <= hi <= size()
     */
    void erase(unsigned long lo, unsigned long hi) {
        data.erase(data.begin() + lo, data.begin() + hi);
        if (lo % BLOCK_SIZE == 0 && hi % BLOCK_SIZE == 0) {
            block_counts.erase(
                    block_counts.begin() + lo / BLOCK_SIZE,
                    block_counts.begin() + hi / BLOCK_SIZE
            );
        } else {
            block_counts.resize((data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE, 0);
            recompute(lo);
        }
    }

    /**
     * Gets the size (number of bits) of this bit vector
     * @return
     */
    unsigned long size() {
        return data.size();
    }

    /**
     * Construct an all-zeros bit vector with the given number of bits
     *
     * @param size the number of bits the constructed vector will contain
     */
    explicit BitVector(unsigned long size) :
            data(size, false),
            block_counts((size + BLOCK_SIZE - 1) / BLOCK_SIZE, 0) {}

    /**
     * Constructs a bit vector from the range [lo, hi) of another bit vector
     *
     * @param from the bit vector to copy a range of bits from
     * @param lo the start of the range of bits to take
     * @param hi the end of the range of bits to take
     */
    BitVector(const BitVector &from, unsigned long lo, unsigned long hi) :
            data(from.data.begin() + lo, from.data.begin() + hi),
            block_counts((hi - lo + BLOCK_SIZE - 1) / BLOCK_SIZE) {
        recompute();
    }

    unsigned long memoryUsage() {
        return sizeof(BitVector) + (data.size() + 7) / 8 + block_counts.size();
    }


private:
    /**
     * Private method to re-compute all the values of block_counts from a certain
     * starting point. Used when inserting or deleting bits
     * @param start the first bit that may have changed and require updating the counters
     */
    void recompute(unsigned long start = 0) {
        for (unsigned long block = start / BLOCK_SIZE; block < block_counts.size(); block++) {
            unsigned long min = block * BLOCK_SIZE;
            unsigned long max = (block + 1) * BLOCK_SIZE;
            if (max > data.size()) {
                max = data.size();
            }
            block_counts[block] = (uint8_t) countOnesRaw(min, max);
        }
    }

    unsigned long countOnesRaw(unsigned long lo, unsigned long hi) {
        unsigned long tot = 0;
        for (unsigned long k = lo; k < hi; k++) {
            tot += data[k];
        }
        return tot;
    }

    unsigned long countBlocks(unsigned long lo, unsigned long hi) {
        unsigned long tot = 0;
        for (unsigned long k = lo; k < hi; k++) {
            tot += block_counts[k];
        }
        return tot;
    }
};


#endif //DK2TREE_BITVECTOR_H
