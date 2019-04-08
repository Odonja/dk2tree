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

The code includes tests with use the GoogleTest library, which is available at https://github.com/google/googletest.
