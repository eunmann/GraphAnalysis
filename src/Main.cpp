#include <stdio.h>
#include "Tests.hpp"
#include "BlockTimer.hpp"
#include <string>
#include <omp.h>

void print_info() {
	printf("OpenMP:\n");
	printf("\tNumber of Processors: %d\n", omp_get_num_procs());
	printf("\tMaximum Threads: %d\n", omp_get_max_threads());
}

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");
	printf("Graph Algorithm Performance Analysis on Persistent Memory Machines\n");
	printf("by Evan Unmann\n");

	print_info();

	size_t alloc_size = 1;

	if (argc > 1) {
		alloc_size = std::stol(std::string(argv[1]));
	}

	alloc_size *= 1e9;

	Tests::graph_test_page_rank();
	Tests::pmem_vs_dram_benchmark(alloc_size);

	return 0;
}