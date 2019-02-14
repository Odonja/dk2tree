//
// Created by hugo on 14-2-19.
//

#ifndef DK2TREE_BINARY_SEARCH_H
#define DK2TREE_BINARY_SEARCH_H

#include <vector>

unsigned long
binarySearch(const std::vector<unsigned long> &v, unsigned long k, bool *found);

bool contains(const std::vector<unsigned long> &v, unsigned long k);

unsigned long insert(std::vector<unsigned long> &v, unsigned long k);

void remove(std::vector<unsigned long> &v, unsigned long k);


#endif //DK2TREE_BINARY_SEARCH_H
