#pragma once

typedef unsigned long size_t;
#include <inttypes.h>
#include <vector>
#include <string>
#include "Graph.hpp"

namespace Benchmark {

	typedef struct Parameters {
		size_t alloc_size = 1000000;
		uint32_t num_vertices = 100000;
		uint32_t min_degree = 1;
		uint32_t max_degree = 200;
		float min_value = 1;
		float max_value = 5;
		uint32_t page_rank_iterations = 100;
		float page_rank_dampening_factor = 0.8;
		uint32_t test_iterations = 10;
		std::string graph_path = "";
		std::string pmem_directory = "/pmem/";
	} Parameters;

	Benchmark::Parameters get_parameters();

	void benchmark_page_rank(const Benchmark::Parameters& tp);
	void benchmark_breadth_first_traversal(const Benchmark::Parameters& tp);
	void benchmark_memory(const Benchmark::Parameters& tp);

	std::vector<std::vector<double>> run_page_rank(const Benchmark::Parameters& tp, const Graph& graph);
	std::vector<std::vector<double>> run_breadth_first_traversal(const Benchmark::Parameters& tp, const Graph& graph, std::vector<uint32_t> start_vertices);
	std::vector<std::vector<double>> run_memory(const Benchmark::Parameters& tp, char* arr, const size_t size);

	std::string get_graph_name(const std::string& graph_path);
}