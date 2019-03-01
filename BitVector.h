//
// Created by hugo on 2-1-19.
//

#ifndef DK2TREE_BITVECTOR_H
#define DK2TREE_BITVECTOR_H

#include <vector>
#include <cstdint>
#include <cassert>
#include <cstdio>
#include "parameters.cpp"

using std::vector;

typedef uint64_t u64;
typedef uint8_t u8;

#define MAX_BIT (((u64) 1) << 63)

static const u8 ONE_BITS[256] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

u64 ones(u64 n) {
    return ONE_BITS[n & 0xFF]
           + ONE_BITS[(n >> 8) & 0xFF]
           + ONE_BITS[(n >> 16) & 0xFF]
           + ONE_BITS[(n >> 24) & 0xFF]
           + ONE_BITS[(n >> 32) & 0xFF]
           + ONE_BITS[(n >> 40) & 0xFF]
           + ONE_BITS[(n >> 48) & 0xFF]
           + ONE_BITS[(n >> 56) & 0xFF];
}

/**
 * A simple bitvector containing the `raw` bits in a vector<bool>, as well as
 * a list of the number of ones in each block, to speed up rank operations
 */
template<unsigned long LENGTH = (B + 63) / 64 + 1>
struct BitVector {
    u64 bits;
    u64 data[LENGTH];
    u8 block_counts[LENGTH];

    /**
     * Gives the value of the n-th bit in the bitvector. This is a read-only operator,
     * since the block_counts must also be updated when writing
     *
     * @param n an index with 0 <= n < bv.size()
     * @return the value of the n-th bit of the bitvector
     */
    const bool operator[](unsigned long n) const {
        u64 idx = n / 64;
        u64 mask = MAX_BIT >> (n % 64);
        return (data[idx] & mask) != 0;
    }

    /**
     * Sets the n-th bit to value b, and returns true if the value changed
     * @param n an index with 0 <= n < size()
     * @param b a boolean
     * @return true iff the previous value of bit n was unequal to b
     */
    const bool set(unsigned long n, bool b) {
        u64 block = n / 64;
        u64 mask = MAX_BIT >> (n % 64);

        bool changed = ((data[block] & mask) != 0) ^b;

        if (changed) {
            if (b) {
                data[block] |= mask;
                block_counts[block]++;
            } else {
                data[block] &= ~mask;
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
        unsigned long end_blocks = n - n % 64;
        unsigned long nr_blocks = end_blocks / 64;
        return countBlocks(0, nr_blocks) + countOnesRaw(end_blocks, n);
    }

    /**
     * Returns the number of 1-bits in the interval [lo, hi), which is equal to
     * rank1(hi) - rank1(lo)
     */
    unsigned long rangeRank1(unsigned long lo, unsigned long hi) {
        unsigned long blockLo = (lo + 64 - 1) / 64, blockHi = hi / 64;
        if (blockLo > blockHi) {
            return countOnesRaw(lo, hi);
        }
        unsigned long blockStart = blockLo * 64, blockEnd = blockHi * 64;
        return countOnesRaw(lo, blockStart) + countBlocks(blockLo, blockHi) +
               countOnesRaw(blockEnd, hi);
    }

    /**
     * Inserts `size` 0-bits at position `begin`
     * @param begin an index with 0 <= begin <= size()
     * @param size the number of bits to be inserted
     */
    void insert(unsigned long begin, unsigned long size) {
        u64 block_start = begin / 64;
        u64 block_amount = size / 64;
        u64 bit_amount = size % 64;

        // We save the first block, so we can set everything but the part to be
        // moved to zero, simplifying the rest
        u64 first_part_mask = (2ULL << (63 - begin % 64)) - 1;
        u64 first_block_keep = data[block_start] & ~first_part_mask;
        data[block_start] &= first_part_mask;

        // First, shift by whole number of blocks if applicable
        if (block_amount != 0) {
            for (u64 idx = LENGTH - 1;
                 idx >= block_start + block_amount; idx--) {
                data[idx] = data[idx - block_amount];
                data[idx - block_amount] = 0;
            }
        }

        // Then, shift by the remaining number of bits
        if (bit_amount != 0) {
            for (u64 idx = LENGTH - 1; idx >= block_start + 1; idx--) {
                data[idx] = (data[idx] >> bit_amount) |
                            (data[idx - 1] << (64 - bit_amount));
            }
            data[block_start] >>= bit_amount;
        }

        // Finally, restore the first block
        // We don't recompute the ones counts, since this method is only used by
        // other methods that will recompute it
        data[block_start] |= first_block_keep;

        bits += size;
        recompute(begin);
    }

    /**
     * Inserts the range [lo, hi) of the bit vector `from` into this bit vector
     *
     * @param begin the position in this bit vector to insert into
     * @param from the bit vector to insert a range from
     * @param lo the start of the range in `from` to insert
     * @param hi the end of the range in `from` to insert
     */
    void
    insert(unsigned long begin, const BitVector<LENGTH> &from, unsigned long lo,
           unsigned long hi) {
        insert(begin, hi - lo);
        for (u64 idx = 0; idx + lo < hi; idx++) {
            set(idx + begin, from[idx + lo]);
        }
        recompute(begin);
    }

    /**
     * Appends the range [lo, hi) of the bit vector `from` into this bit vector
     *
     * @param from the bit vector to append bits from
     * @param lo the start of the range of bits to append
     * @param hi the end of the range of bits to append
     */
    void append(const BitVector &from, unsigned long lo, unsigned long hi) {
        insert(bits, from, lo, hi);
    }


    /**
     * Deletes the indicated range of bits starting at the indicated index
     * @param lo an index with 0 <= lo <= size()
     * @param hi the end of the range of bits to be deleted. Should satisfy lo <= hi <= size()
     */
    void erase(unsigned long lo, unsigned long hi) {
        u64 amount = hi - lo;
        u64 block_start = lo / 64;
        u64 block_amount = amount / 64;
        u64 bit_amount = amount % 64;

        // We save the first block, so we can set everything but the part to be
        // moved to zero, simplifying the rest
        u64 first_part_mask = (2ULL << (63 - lo % 64)) - 1;
        u64 first_block_keep = data[block_start] & ~first_part_mask;
        data[block_start] &= first_part_mask;

        // First, move everything over by the specified number of blocks
        if (block_amount != 0) {
            for (u64 idx = block_start; idx + block_amount < LENGTH; idx++) {
                data[idx] = data[idx + block_amount];
                data[idx + block_amount] = 0;
            }
        }

        // Then, shift everything over by the correct bit-amount
        if (bit_amount != 0) {
            for (u64 idx = block_start; idx + 1 < LENGTH; idx++) {
                data[idx] = (data[idx] << bit_amount) |
                            (data[idx + 1] >> (64 - bit_amount));
            }
            data[LENGTH - 1] <<= bit_amount;
        }

        // Finally, fix the first block and recompute
        data[block_start] &= first_part_mask;
        data[block_start] |= first_block_keep;

        bits -= amount;
        recompute(lo);
    }

    /**
     * Gets the size (number of bits) of this bit vector
     * @return
     */
    unsigned long size() {
        return bits;
    }

    /**
     * Construct an all-zeros bit vector with the given number of bits
     *
     * @param size the number of bits the constructed vector will contain
     */
    explicit BitVector(unsigned long size) :
            bits(size),
            data{0},
            block_counts{0} {}

    /**
     * Constructs a bit vector from the range [lo, hi) of another bit vector
     *
     * @param from the bit vector to copy a range of bits from
     * @param lo the start of the range of bits to take
     * @param hi the end of the range of bits to take
     */
    BitVector(const BitVector &from, unsigned long lo, unsigned long hi) :
            bits(0),
            data{0},
            block_counts{0} {
        insert(0, from, lo, hi);
    }

    unsigned long memoryUsage() {
        // For each 64-bit block, we store a 64-bit integer (containing those
        // bits), and an 8-bit integer storing the number of one-bits
        // So 72 bits = 9 bytes for each block
        return ((bits + 63) / 64) * 9;
    }


private:
    /**
     * Private method to re-compute all the values of block_counts from a certain
     * starting point. Used when inserting or deleting bits
     * @param start the first bit that may have changed and require updating the counters
     */
    void recompute(unsigned long start = 0) {
        start /= 64;
        for (u64 block = start; block < LENGTH; block++) {
            block_counts[block] = (u8) ones(data[block]);
        }
    }

    unsigned long countOnesRaw(unsigned long lo, unsigned long hi) {
        u64 block = lo / 64;
        lo -= block * 64;
        hi -= block * 64;

        u64 mask = ((1ULL << (hi - lo)) - 1) << (64 - hi);
        return ones(data[block] & mask);
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
