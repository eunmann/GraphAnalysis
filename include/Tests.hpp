#pragma once

typedef unsigned long size_t;
#include <inttypes.h>

namespace Tests {
	void PMEM_tests();

	void graph_test();

	void graph_test_page_rank(const uint32_t num_vertices);

	void graph_test_breadth_first_traversal(const uint32_t num_vertices);

	void memory_benchmark(char* arr, const size_t size);

	void pmem_vs_dram_benchmark(const size_t alloc_size);
}