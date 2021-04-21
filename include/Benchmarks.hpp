#pragma once

#include <cstddef>
#include <inttypes.h>
#include <vector>
#include <string>
#include <iostream>

#include "GraphCRS.hpp"
#include "BlockTimer.hpp"
#include "GraphAlgorithms.hpp"
#include "PMEM/allocator.hpp"
#include "BenchmarkUtils.hpp"

namespace Benchmark {

	enum BFS_DIRECTION {
		TOP_DOWN,
		BOTTOM_UP,
		HYBRID
	};

	const char* enum_to_string(BFS_DIRECTION direction);

	typedef struct Parameters {
		size_t mem_alloc_size = 1000000;
		uint32_t graph_num_vertices = 100000;
		uint32_t graph_min_degree = 1;
		uint32_t graph_max_degree = 200;
		float graph_min_value = 1;
		float graph_max_value = 5;
		uint32_t page_rank_iterations = 100;
		uint32_t page_rank_num_dampening_factors = 4;
		std::vector<size_t> page_rank_dampening_factors = { 1,2,4,8,16,32,64 };
		BFS_DIRECTION bfs_direction = BFS_DIRECTION::HYBRID;
		uint32_t test_iterations = 10;
		std::string graph_path = "";
		std::string graph_name = "";
		std::string out_dir = "";
	} Parameters;

	Benchmark::Parameters get_parameters();

	void benchmark_page_rank(const Benchmark::Parameters& tp, const GraphCRS<std::allocator>& graph_dram, const GraphCRS<PMEM::allocator>& graph_pmem);
	void benchmark_page_rank_sizes(const Benchmark::Parameters& tp, const GraphCRS<std::allocator>& graph_dram, const GraphCRS<PMEM::allocator>& graph_pmem);
	void benchmark_breadth_first_traversal(const Benchmark::Parameters& tp, const GraphCRS<std::allocator>& graph_dram, const GraphCRS<PMEM::allocator>& graph_pmem);
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
		printf("Number of Page Ranks: %u\n", tp.page_rank_num_dampening_factors);
		printf("Number of Iterations: %u\n", tp.page_rank_iterations);
		printf("Iteration, Time Elapsed (s), MTEPS\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			GraphAlgorithms::page_rank_v_neighbors<alloc_type, temp_alloc_type>(graph, tp.page_rank_iterations, std::vector<float>(tp.page_rank_num_dampening_factors, 0.8f));
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
		printf("Direction: %s\n", Benchmark::enum_to_string(tp.bfs_direction));
		printf("Function Templates: %s\n", BenchmarkUtils::parse_templates_from_signatures(__PRETTY_FUNCTION__).c_str());
		printf("Iteration, Time Elapsed (s), MTEPS\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;

			std::vector<int32_t, temp_alloc_type<int32_t>> vertex_depth;
			switch (tp.bfs_direction) {
			case BFS_DIRECTION::HYBRID:
				vertex_depth = GraphAlgorithms::breadth_first_traversal_hybrid<alloc_type, temp_alloc_type>(graph, start_vertices[iter - 1]);
				break;
			case BFS_DIRECTION::TOP_DOWN:
				vertex_depth = GraphAlgorithms::breadth_first_traversal_top_down<alloc_type, temp_alloc_type>(graph, start_vertices[iter - 1]);
				break;
			case BFS_DIRECTION::BOTTOM_UP:
				vertex_depth = GraphAlgorithms::breadth_first_traversal_bottom_up<alloc_type, temp_alloc_type>(graph, start_vertices[iter - 1]);
				break;
			}

			timer.end();

			size_t num_vertices_checked = 0;
			size_t num_edges_traversed = 0;

#pragma omp parallel for schedule(static) reduction(+:num_vertices_checked,num_edges_traversed)
			for (size_t vertex = 0; vertex < vertex_depth.size(); vertex++) {
				if (vertex_depth[vertex] != -1) {
					num_vertices_checked++;
					num_edges_traversed += graph.num_neighbors(vertex);
				}
			}

			/*
			*	If a graph is disconnected, we might have selected a source vertex
			*	that is not part of the main graph. If we check the vertex depth,
			*	we can confirm "enough" vertices have been traversed.
			*/
			if (num_vertices_checked < graph.num_vertices() * 0.25 || num_edges_traversed < graph.num_edges() * 0.25) {
				start_vertices[iter - 1]++;
				iter--;
				continue;
			}

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			double edges_per_second = num_edges_traversed / time_elapsed_seconds;

			mteps_metrics.push_back(edges_per_second / 1e6);
			printf("%u, %.3f, %.3f\n", iter, time_elapsed_seconds, mteps_metrics.back());
		}

		BenchmarkUtils::print_metrics("MTEPS", mteps_metrics);

		return mteps_metrics;
	}

}