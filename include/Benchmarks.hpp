#pragma once

#include <cstddef>
#include <inttypes.h>
#include <vector>
#include <string>
#include <iostream>

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
		std::string graph_name = "";
		std::string pr_csv_path = "";
		std::string bfs_csv_path = "";
		std::string mem_csv_path = "";
	} Parameters;

	Benchmark::Parameters get_parameters();

	void benchmark_page_rank(const Benchmark::Parameters& tp);
	void benchmark_breadth_first_traversal(const Benchmark::Parameters& tp);
	void benchmark_memory(const Benchmark::Parameters& tp);
	void benchmark_STREAM(const Benchmark::Parameters& tp);

	std::vector<std::vector<double>> run_memory(const Benchmark::Parameters& tp, char* arr, size_t size);

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<double> run_page_rank(const Benchmark::Parameters& tp, const GraphCRS<alloc_type>& graph) {
		BlockTimer timer("Page Rank");
		printf("Page Rank\n");

		std::vector<double> mteps_metrics;

		printf("Number of Vertices: %lu = %s\n", graph.num_vertices(), FormatUtils::format_number(graph.num_vertices()).c_str());
		printf("Number of Edges: %lu = %s\n", graph.num_edges(), FormatUtils::format_number(graph.num_edges()).c_str());
		printf("Memory Size: %lu = %sB\n", graph.byte_size(), FormatUtils::format_number(graph.byte_size()).c_str());
		printf("Function Templates: %s\n", BenchmarkUtils::parse_templates_from_signatures(__PRETTY_FUNCTION__).c_str());
		printf("Iteration, Time Elapsed (s), MTEPS\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			GraphAlgorithms::page_rank_2<alloc_type, temp_alloc_type>(graph, tp.page_rank_iterations, std::vector<float>(tp.num_dampening_factors, tp.page_rank_dampening_factor));
			timer.end();

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			mteps_metrics.push_back(((tp.page_rank_iterations * graph.num_edges()) / time_elapsed_seconds) / 1e6);
			printf("%u, %.3f, %.3f\n", iter, time_elapsed_seconds, mteps_metrics.back());
		}

		BenchmarkUtils::print_metrics("MTEPS", mteps_metrics);

		return mteps_metrics;
	}

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<double> run_breadth_first_traversal(const Benchmark::Parameters& tp, const GraphCRS<alloc_type>& graph, std::vector<uint32_t>& start_vertices) {
		BlockTimer timer("Breadth First Traversal");
		printf("Breadth First Traversal\n");
		std::vector<double> mteps_metrics;

		printf("Number of Vertices: %lu = %s\n", graph.num_vertices(), FormatUtils::format_number(graph.num_vertices()).c_str());
		printf("Number of Edges: %lu = %s\n", graph.num_edges(), FormatUtils::format_number(graph.num_edges()).c_str());
		printf("Memory Size: %lu = %sB\n", graph.byte_size(), FormatUtils::format_number(graph.byte_size()).c_str());
		printf("Function Templates: %s\n", BenchmarkUtils::parse_templates_from_signatures(__PRETTY_FUNCTION__).c_str());
		printf("Iteration, Time Elapsed (s), MTEPS\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			std::vector<int32_t, temp_alloc_type<int32_t>> vertex_depth = GraphAlgorithms::breadth_first_traversal_hybrid<alloc_type, temp_alloc_type>(graph, start_vertices[iter - 1]);
			timer.end();

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			double edges_per_second = graph.num_edges() / time_elapsed_seconds;

			/*
			*	If a graph is disconnected, we might have selected a source vertex
			*	that is not part of the main graph. If we check the number of -1 vertex
			*   depths, we can see how many vertices were actually checked. If its too
			*	low, then we run again.
			*/
			size_t num_vertices_traversed = 0;

#pragma omp parallel for schedule(static) reduction(+:num_vertices_traversed)
			for (size_t i = 0; i < vertex_depth.size(); i++) {
				if (vertex_depth[i] != -1) {
					num_vertices_traversed++;
				}
			}

			if (num_vertices_traversed < graph.num_vertices() * 0.5) {
				start_vertices[iter - 1]++;
				iter--;
				continue;
			}

			mteps_metrics.push_back(edges_per_second / 1e6);
			printf("%u, %.3f, %.3f\n", iter, time_elapsed_seconds, mteps_metrics.back());
		}

		BenchmarkUtils::print_metrics("MTEPS", mteps_metrics);

		return mteps_metrics;
	}

}