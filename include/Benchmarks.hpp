#pragma once

#include <cstddef>
#include <inttypes.h>
#include <vector>
#include <string>

#include "GraphCRS.hpp"
#include "BlockTimer.hpp"
#include "GraphAlgorithms.hpp"
#include "BenchmarkUtils.hpp"

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
		uint32_t num_dampening_factors = 4;
		uint32_t test_iterations = 10;
		std::string graph_path = "";
	} Parameters;

	Benchmark::Parameters get_parameters();

	void benchmark_page_rank(const Benchmark::Parameters& tp);
	void benchmark_breadth_first_traversal(const Benchmark::Parameters& tp);
	void benchmark_memory(const Benchmark::Parameters& tp);

	std::vector<std::vector<double>> run_memory(const Benchmark::Parameters& tp, char* arr, size_t size);

	template<template<class> class T>
	std::vector<std::vector<double>> run_page_rank(const Benchmark::Parameters& tp, const GraphCRS<T>& graph) {
		BlockTimer timer("Page Rank");
		printf("Page Rank\n");

		std::vector<std::vector<double>> metrics(2);
		std::vector<double>& time_elapsed_v = metrics[0];
		std::vector<double>& edges_per_second_v = metrics[1];

		printf("Number of Vertices: %lu\n", graph.num_vertices());
		printf("Number of Edges: %lu\n", graph.num_edges());
		printf("Memory Size: %lu B\n", graph.byte_size());
		printf("Iteration, Time Elapsed (s), Edges per Second\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			GraphAlgorithms::page_rank(graph, tp.page_rank_iterations, std::vector<float>(tp.num_dampening_factors, tp.page_rank_dampening_factor));
			timer.end();

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			time_elapsed_v.push_back(time_elapsed_seconds);
			edges_per_second_v.push_back(tp.page_rank_iterations * graph.num_edges() / time_elapsed_seconds);
			printf("%u, %.3f, %.3f\n", iter, time_elapsed_seconds, edges_per_second_v.back());
		}

		BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);
		BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);

		return metrics;
	}

	template<template<class> class T>
	std::vector<std::vector<double>> run_breadth_first_traversal(const Benchmark::Parameters& tp, const GraphCRS<T>& graph, std::vector<uint32_t> start_vertices) {
		BlockTimer timer("Breadth First Traversal");
		printf("Breadth First Traversal\n");
		std::vector<std::vector<double>> metrics(2);

		std::vector<double>& time_elapsed_v = metrics[0];
		std::vector<double>& edges_per_second_v = metrics[1];

		printf("Number of Vertices: %lu\n", graph.num_vertices());
		printf("Number of Edges: %lu\n", graph.num_edges());
		printf("Memory Size: %lu B\n", graph.byte_size());
		printf("Iteration, Time Elapsed (s), Edges per Second\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			GraphAlgorithms::breadth_first_traversal(graph, start_vertices[iter - 1]);
			timer.end();

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			time_elapsed_v.push_back(time_elapsed_seconds);
			edges_per_second_v.push_back(graph.num_edges() / time_elapsed_seconds);
			printf("%u, %.3f, %.3f\n", iter, time_elapsed_seconds, edges_per_second_v.back());
		}

		BenchmarkUtils::print_metrics("Time Elapsed (s)", time_elapsed_v);
		BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);

		return metrics;
	}

}