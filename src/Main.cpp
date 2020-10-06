#include <stdio.h>

#include "Graph.hpp"
#include "Timer.hpp"

int main(int argc, char** argv) {
    Timer timer("Time Elapsed");
    printf("Graph Analysis for a Graph Algorithm on Persistent Memory Machines\n");
    printf("by Evan Unmann\n");

    Graph g(10);

    g.forEach([&g](uint32_t& v, const uint32_t i, const uint32_t j) {
        v = g.numOfNodes * i + j;
    });

    g.forEach([](uint32_t& v, const uint32_t i, const uint32_t j) {
        printf("Value: %u\n", v);
    });

    timer.end();
    timer.print();

    return 0;
}