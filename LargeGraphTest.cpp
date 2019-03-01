//
// Created by hugo on 14-2-19.
//

#ifndef DK2TREE_LARGE_GRAPH_TEST
#define DK2TREE_LARGE_GRAPH_TEST

#include <chrono>
#include <random>
#include <functional>
#include <thread>

std::default_random_engine generator(
        (unsigned long) std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_int_distribution<unsigned long> distribution(0,
                                                          0xFFFFFFFFFFFFFFFF);

unsigned long randRange(unsigned long lo, unsigned long hi) {
    return lo + distribution(generator) % (hi - lo);
}

class Timer {
    std::chrono::steady_clock::time_point t0, t1;
public:
    void start() {
        t0 = std::chrono::steady_clock::now();
    }

    void stop() {
        t1 = std::chrono::steady_clock::now();
    }

    double read() {
        std::chrono::duration<double, std::nano> d(t1 - t0);
        return d.count() / 1000000000.0;
    }
};

template<typename G>
G *largeTest(unsigned long size, unsigned long m, bool verbose = false) {
    Timer t;
    t.start();
    G *g = G::withSize(size);
    t.stop();
    printf("Initialisation: %f seconds\n", t.read());
    if (verbose) {
        t.start();
        for (unsigned long k = 1; k <= m; k++) {
            if (k % 10000 == 0) {
                printf("%lu / %lu\r", k, m);
            }
            unsigned long i = randRange(0, size);
            unsigned long j = randRange(0, size);
            g->addEdge(i, j);
        }
        t.stop();
    } else {
        t.start();
        for (unsigned long k = 1; k <= m; k++) {
            unsigned long i = randRange(0, size);
            unsigned long j = randRange(0, size);
            g->addEdge(i, j);
        }
        t.stop();
    }
    printf("Adding edges: %f seconds\n", t.read());
    printf("Size in memory: %lu bytes\n", g->memoryUsage());
    unsigned long i = randRange(0, size);
    unsigned long j = randRange(0, size);
    printf("%i\n", g->reportEdge(i, j));
    return g;
}

#endif // DK2TREE_LARGE_GRAPH_TEST