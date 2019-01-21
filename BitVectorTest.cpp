//
// Created by hugo on 2-1-19.
//

#ifndef BIT_VECTOR_TEST
#define BIT_VECTOR_TEST

#include "BitVector.h"

TEST(BitVectorTest, ReadWrite) {
    unsigned long size = 512;
    BitVector bv(size);
    unsigned long toFlip[] = {
            0, 1, 8, 9, 12, 21, 30, 35, 38, 50, 51, 63, 65, 66, 73, 74,
            83, 89, 100, 106, 107, 108, 109, 110, 120, 132, 148, 149, 153, 165, 167, 175,
            176, 179, 180, 181, 184, 185, 186, 192, 194, 197, 200, 201, 204, 211, 213, 215,
            218, 229, 246, 247, 248, 249, 250, 255, 264, 266, 268, 270, 271, 276, 277, 281,
            285, 286, 287, 301, 304, 307, 308, 326, 327, 328, 329, 330, 334, 342, 343, 345,
            346, 347, 350, 351, 359, 360, 361, 364, 368, 370, 371, 377, 379, 381, 386, 388,
            393, 397, 400, 402, 404, 409, 412, 422, 423, 424, 431, 434, 436, 447, 448, 449,
            455, 457, 466, 471, 474, 478, 484, 488, 490, 491, 492, 497, 506, 507, 508, 510
    };

    for (unsigned long i : toFlip) {
        bv.set(i, true);
    }

    unsigned long i = 0;
    unsigned long idx = 0;
    for (unsigned long j : toFlip) {
        // All bits in between two entries of toFlip should be zero
        while (i < j) {
            EXPECT_FALSE(bv[i]);
            EXPECT_EQ(bv.rank1(i), idx);
            i++;
        }
        // The entry in toFlip should be one
        EXPECT_TRUE(bv[i]);
        EXPECT_EQ(bv.rank1(i), idx);
        idx++;
        i++;
    }
}

TEST(BitVectorTest, InsertDelete) {
    unsigned long size = 512;
    BitVector bv(size);
    bv.set(100, true);
    EXPECT_EQ(bv.rank1(100), 0);
    EXPECT_EQ(bv.rank1(101), 1);

    bv.erase(75, 25);
    EXPECT_EQ(bv.size(), size - 25);
    EXPECT_TRUE(bv[75]);
    EXPECT_FALSE(bv[100]);
    EXPECT_EQ(bv.rank1(75), 0);
    EXPECT_EQ(bv.rank1(76), 1);
    EXPECT_EQ(bv.rank1(100), 1);

    bv.insert(76, 100);
    EXPECT_EQ(bv.size(), size + 75);
    EXPECT_EQ(bv.rank1(75), 0);
    EXPECT_EQ(bv.rank1(76), 1);
    EXPECT_EQ(bv.rank1(100), 1);

    bv.insert(60, 25);
    EXPECT_EQ(bv.size(), size + 100);
    EXPECT_EQ(bv.rank1(100), 0);
    EXPECT_EQ(bv.rank1(101), 1);
    EXPECT_EQ(bv.rank1(125), 1);
}

/**
 * For many values of k, create a bit vector with 1's every k bits
 * Then verify that all access and rank operations are correct
 */
TEST(BitVectorTest, Patterns) {
    unsigned long size = 512;
    for (unsigned long k = 1; k < size; k++) {
        BitVector bv(size);
        for (unsigned long i = k - 1; i < size; i += k) {
            bv.set(i, true);
        }
        for (unsigned long i = 0; i < size; i++) {
            EXPECT_EQ(bv[i], ((i + 1) % k == 0));
            EXPECT_EQ(bv.rank1(i), i / k);
        }
    }
}

#endif // BIT_VECTOR_TEST