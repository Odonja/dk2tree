//
// Created by hugo on 2-1-19.
//

#ifndef DK2TREE_BITVECTOR_H
#define DK2TREE_BITVECTOR_H

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

    /**
     * Gives the value of the n-th bit in the bitvector. This is a read-only operator,
     * since the block_counts must also be updated when writing
     *
     * @param n an index with 0 <= n < bv.size()
     * @return the value of the n-th bit of the bitvector
     */
    const bool operator[](unsigned long n);

    /**
     * Sets the n-th bit to value b, and returns true if the value changed
     * @param n an index with 0 <= n < size()
     * @param b a boolean
     * @return true iff the previous value of bit n was unequal to b
     */
    const bool set(unsigned long n, bool b);

    /**
     * Performs the rank-operation on this bitvector
     * @param n an index with 0 <= n <= size()
     * @return the number of ones in the bits [0 ... n)
     */
    unsigned long rank1(unsigned long n);

    /**
     * Returns the number of 1-bits in the interval [lo, hi), which is equal to
     * rank1(hi) - rank1(lo)
     */
    unsigned long rangeRank1(unsigned long lo, unsigned long hi);

    /**
     * Inserts `size` 0-bits at position `begin`
     * @param begin an index with 0 <= begin <= size()
     * @param size the number of bits to be inserted
     */
    void insert(unsigned long begin, unsigned long size);

    /**
     * Inserts the range [lo, hi) of the bit vector `from` into this bit vector
     *
     * @param begin the position in this bit vector to insert into
     * @param from the bit vector to insert a range from
     * @param lo the start of the range in `from` to insert
     * @param hi the end of the range in `from` to insert
     */
    void insert(unsigned long begin, const BitVector &from, unsigned long lo, unsigned long hi);

    /**
     * Appends the range [lo, hi) of the bit vector `from` into this bit vector
     *
     * @param from the bit vector to append bits from
     * @param lo the start of the range of bits to append
     * @param hi the end of the range of bits to append
     */
    void append(const BitVector &from, unsigned long lo, unsigned long hi);

    /**
     * Deletes the indicated range of bits starting at the indicated index
     * @param lo an index with 0 <= lo <= size()
     * @param hi the end of the range of bits to be deleted. Should satisfy lo <= hi <= size()
     */
    void erase(unsigned long lo, unsigned long hi);

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

private:
    /**
     * Private method to re-compute all the values of block_counts from a certain
     * starting point. Used when inserting or deleting bits
     * @param start the first bit that may have changed and require updating the counters
     */
    void recompute(unsigned long start = 0);
};


#endif //DK2TREE_BITVECTOR_H
