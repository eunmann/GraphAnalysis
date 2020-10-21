#include <libpmemobj.h>
#include <stdio.h>

#include "Graph.hpp"
#include "PMEMTest.hpp"
#include "Timer.hpp"

int main(int argc, char** argv) {
    Timer timer("Time Elapsed");
    printf("Graph Analysis for a Graph Algorithm on Persistent Memory Machines\n");
    printf("by Evan Unmann\n");

    // PMEMTest::simpleStructWrite();
    // PMEMTest::simpleStructRead();

    timer.end();
    timer.print();

    return 0;
}