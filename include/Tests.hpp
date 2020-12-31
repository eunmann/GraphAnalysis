#pragma once

typedef unsigned long size_t;
#include <inttypes.h>
#include <vector>

namespace Tests {

	typedef struct TestParameters {
		size_t alloc_size = 1000000;
		uint32_t num_vertices = 100000;
		uint32_t min_degree = 1;
		uint32_t max_degree = 200;
		float min_value = 1;
		float max_value = 5;
		uint32_t page_rank_iterations = 100;
		float page_rank_dampening_factor = 0.8;
		uint32_t test_iterations = 10;
	} TestParameters;

	Tests::TestParameters get_test_parameters();

	void PMEM_tests();

	void graph_test(const Tests::TestParameters& tp);

	void graph_test_page_rank(const Tests::TestParameters& tp);

	void graph_test_breadth_first_traversal(const Tests::TestParameters& tp);

	std::vector<std::vector<double>> memory_benchmark(char* arr, const size_t size);

	void pmem_vs_dram_benchmark(const Tests::TestParameters& tp);
}