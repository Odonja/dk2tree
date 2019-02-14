//
// Created by hugo on 14-2-19.
//

#ifndef DK2TREE_LARGE_GRAPH_TEST
#define DK2TREE_LARGE_GRAPH_TEST

#include <chrono>
#include <random>
#include <functional>
#include <thread>

std::default_random_engine generator((unsigned long) std::chrono::steady_clock::now().time_since_epoch().count());
std::uniform_real_distribution<double> distribution(0.0, 1.0);

static auto randomUniform = std::bind(distribution, generator);

bool bernoulli(double p) {
    return randomUniform() < p;
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

template <typename G>
void largeTest(unsigned long size, double p) {
    printf("Size: %lu, p: %f\n", size, p);

    Timer t;
    t.start();
    G g;
    t.stop();
    printf("Initialisation: %f seconds\n", t.read());
    t.start();
    for (unsigned long k = 0; k < size; k++) {
        g.insertEntry();
    }
    t.stop();
    printf("Inserting entries: %f seconds\n", t.read());
    t.start();
    unsigned long n = 0;
    for (unsigned long i = 0; i < size; i++) {
//        printf("%lu\n", i);
        for (unsigned long j = 0; j < size; j++) {
            if (bernoulli(p)) {
                g.addEdge(i, j);
                n++;
            }
        }
    }
    t.stop();
    printf("Adding edges: %f seconds\n", t.read());
    printf("Size in memory: %lu bytes\n", g.memoryUsage());
    printf("Edges: %lu\n", n);
}

#endif // DK2TREE_LARGE_GRAPH_TEST