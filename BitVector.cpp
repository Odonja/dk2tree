//
// Created by hugo on 2-1-19.
//

#include <cstdio>
#include "BitVector.h"

const bool BitVector::operator[](unsigned long n) {
    return data[n];
}

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

unsigned long BitVector::rank1(unsigned long n) {
    unsigned long end_blocks = n - n % BLOCK_SIZE;
    unsigned long nr_blocks = end_blocks / BLOCK_SIZE;
    return countBlocks(0, nr_blocks) + countOnesRaw(end_blocks, n);
}

unsigned long BitVector::rangeRank1(unsigned long lo, unsigned long hi) {
    unsigned long blockLo = (lo + BLOCK_SIZE - 1) / BLOCK_SIZE, blockHi = hi / BLOCK_SIZE;
    if (blockLo > blockHi) {
        return countOnesRaw(lo, hi);
    }
    unsigned long blockStart = blockLo * BLOCK_SIZE, blockEnd = blockHi * BLOCK_SIZE;
    return countOnesRaw(lo, blockStart) + countBlocks(blockLo, blockHi) + countOnesRaw(blockEnd, hi);
}

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

void BitVector::erase(unsigned long lo, unsigned long hi) {
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

void BitVector::recompute(unsigned long start) {
    for (unsigned long block = start / BLOCK_SIZE; block < block_counts.size(); block++) {
        unsigned long min = block * BLOCK_SIZE;
        unsigned long max = (block + 1) * BLOCK_SIZE;
        if (max > data.size()) {
            max = data.size();
        }
        block_counts[block] = (uint8_t) countOnesRaw(min, max);
    }
}

unsigned long BitVector::countOnesRaw(unsigned long lo, unsigned long hi) {
    unsigned long tot = 0;
    for (unsigned long k = lo; k < hi; k++) {
        tot += data[k];
    }
    return tot;
}

unsigned long BitVector::countBlocks(unsigned long lo, unsigned long hi) {
    unsigned long tot = 0;
    for (unsigned long k = lo; k < hi; k++) {
        tot += block_counts[k];
    }
    return tot;
}