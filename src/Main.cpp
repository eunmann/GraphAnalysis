#include <stdio.h>
#include "Tests.hpp"
#include "BlockTimer.hpp"
#include <string>
#include <omp.h>
#include <exception>
#include <cstdlib>
#include "FormatUtils.hpp"

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
	uint32_t num_vertices = 1;

	if (std::getenv("alloc_size") != nullptr) {
		alloc_size = std::stol(std::getenv("alloc_size"));
	}

	if (std::getenv("num_vertices") != nullptr) {
		num_vertices = std::stol(std::getenv("num_vertices"));
	}

	printf("Memory Test:\n");
	printf("\talloc_size: %sB\n", FormatUtils::format_number(alloc_size).c_str());

	printf("Graph Test:\n");
	printf("\tnum_vertices: %s\n", FormatUtils::format_number(num_vertices).c_str());

	try {
		Tests::graph_test_page_rank(num_vertices);
		Tests::pmem_vs_dram_benchmark(alloc_size);
	}
	catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}

	return 0;
}