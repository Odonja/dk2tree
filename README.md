# dk2tree

A C++ implementation of the data structure described in *N. R. Brisaboa, A. Cerdeira-Pena, G. de Bernardo, and G. Navarro. Compressed representation of dynamic binary relations with applications.Information Systems, 69:106–123, 2017.*

Features
- [x] Efficient TTree and LTree representations using B+trees
- [x] Space-efficient size-limited dynamic bitvectors with rank-support
- [x] Faster lookups using auxiliary tree traversal arrays
- [x] Check existence of given edge
- [x] Report all edges between vertices in given range
- [ ] Extra compression with matrix vocabulary
- [ ] Report all successors/predecessors of given vertex
- [ ] Efficient bulk-loading from large file
- [ ] Different values of ```k``` for top/bottom parts of trees

The code includes tests with use the GoogleTest library, which is available at https://github.com/google/googletest.

## Building

The project contains a ```CMakeLists.txt``` which allows easily building it using CMake. This will produce two binaries:

- ```dktree_test```, which executes all tests using GoogleTest
- ```dktree```, which can be used to run benchmarks

## Class overview

The *dk²-tree* consists of four data structures

### BitVector

The *BitVector* is a simple implementation of a bitvector with bounded size, and efficient support for the *rank* operation (where *rank(k)* is the number of 1-bits in the first *k* bits of the bitvector). The maximum size of the bitvector is a template argument, but the default value is always correct with respect to the maximum leaf size specified in ```parameters.cpp```. A BitVector consists of two arrays: one of 64-bit words representing the actual bit array, and one of 8-bit values counting the number of 1-bits in the corresponding word in the bit array. This way, the *rank* operation can be done by simply iterating over all 8-bit one counts, and then counting the number of 1-bits in the relevant part of the remaining word, which can be done easily using a bitmask and a look-up table.

Note that the bitvector supports get/set/insert/delete, but operations whose indices are out of range or otherwise invalid are not tested and are undefined behaviour.

### TTree

The *TTree* is the basic tree data structure whose leaves form a large bitvector. Here, it is implemented as a 2-3 tree which guarantees asymptotically optimal running time for many operations. It supports the same set, get, and rank operations as the simple bitvector, but only allows insertions and deletions to happen on whole blocks of k² bits at a time.

### LTree

The LTree is a modified version of the TTree, which stores slightly less data since it does not need to support the rank operation. The memory savings are small, though, so a TTree could be used instead without significantly increasing memory usage.

### DKTree

The DKTree is the main class representing a graph database, and supporting graph operations: these operations are implemented as described in Brisaboa et al.'s paper. It supports adding/deleting/querying individual edges, as well as adding and removing vertices. The indices of previously deleted vertices are automatically reused for adding vertices later on.

## Limitations

Currently, the main limitation is that there is no efficient way to load a graph into memory form another compressed format, i.e. this has to be done by adding each edge manually. This severely limits for how large datasets testing is still possible.

Furthermore, *k* is required to be a power of 2, since the insert/delete operations on the TTree and LTree will not work properly otherwise.
