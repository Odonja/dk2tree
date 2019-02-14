//
// Created by hugo on 14-2-19.
//

#include "BinarySearch.h"

unsigned long binarySearch(const std::vector<unsigned long> &v, unsigned long k, bool *found) {
    *found = false;
    unsigned long lo = 0, hi = v.size();
    while (lo < hi) {
        unsigned long mid = (lo + hi) / 2;
        if (v[mid] == k) {
            *found = true;
            return mid;
        } else if (v[mid] < k) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    // lo == hi == the index where the element would be if it was contained in the vector
    return lo;
}

bool contains(const std::vector<unsigned long> &v, unsigned long k) {
    bool found = false;
    binarySearch(v, k, &found);
    return found;
}

unsigned long insert(std::vector<unsigned long> &v, unsigned long k) {
    bool found = false;
    unsigned long idx = binarySearch(v, k, &found);
    if (!found) {
        v.insert(v.begin() + idx, k);
    }
    return idx;
}

void remove(std::vector<unsigned long> &v, unsigned long k) {
    bool found = false;
    unsigned long idx = binarySearch(v, k, &found);
    if (found) {
        v.erase(v.begin() + idx);
    }
}
