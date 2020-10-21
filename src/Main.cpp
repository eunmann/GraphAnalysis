#include <stdio.h>

#include "Graph.hpp"
#include "PMEMTest.hpp"
#include "Timer.hpp"

int main(int argc, char** argv) {
    Timer timer("Time Elapsed");
    printf("Graph Analysis for a Graph Algorithm on Persistent Memory Machines\n");
    printf("by Evan Unmann\n");

    printf("First test, simple struct write and read.\n");
    PMEMTest::simpleStructWrite();
    PMEMTest::simpleStructRead();

    printf("Second test, simple struct write and read with type safety.\n");
    PMEMTest::simpleStructWrite2();
    PMEMTest::simpleStructRead2();

    timer.end();
    timer.print();

    return 0;
}