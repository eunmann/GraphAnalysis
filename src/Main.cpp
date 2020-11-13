#include <stdio.h>
#include "Tests.hpp"
#include "BlockTimer.hpp"
#include <string>

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");
	printf("Graph Algorithm Performance Analysis on Persistent Memory Machines\n");
	printf("by Evan Unmann\n");

	size_t alloc_size = 1 * 1e9;

	if (argc > 1) {
		alloc_size = std::stol(std::string(argv[1]));
	}

	Tests::pmem_vs_dram_benchmark(alloc_size);

	return 0;
}