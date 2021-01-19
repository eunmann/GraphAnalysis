#include "PMEM/Tests.hpp"
#include <stdio.h>
#include "GraphUtils.hpp"
#include <random>
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include "FormatUtils.hpp"
#include "BenchmarkUtils.hpp"
#include "GNUPlot/Plot.hpp"
#include "Benchmarks.hpp"
#include <functional>
#include <immintrin.h>

#define GNUPLOT_WIDTH 800
#define GNUPLOT_HEIGHT 600

namespace Benchmark {

	Benchmark::Parameters get_parameters() {
		Parameters tp;

		if (std::getenv("alloc_size") != nullptr) {
			tp.alloc_size = std::stol(std::getenv("alloc_size"));
		}

		if (std::getenv("num_vertices") != nullptr) {
			tp.num_vertices = std::stol(std::getenv("num_vertices"));
		}

		if (std::getenv("min_degree") != nullptr) {
			tp.min_degree = std::stol(std::getenv("min_degree"));
		}

		if (std::getenv("max_degree") != nullptr) {
			tp.max_degree = std::stol(std::getenv("max_degree"));
		}

		if (std::getenv("min_value") != nullptr) {
			tp.min_value = std::stol(std::getenv("min_value"));
		}

		if (std::getenv("max_value") != nullptr) {
			tp.max_value = std::stol(std::getenv("max_value"));
		}

		if (std::getenv("page_rank_iterations") != nullptr) {
			tp.page_rank_iterations = std::stol(std::getenv("page_rank_iterations"));
		}

		if (std::getenv("page_rank_dampening_factor") != nullptr) {
			tp.page_rank_dampening_factor = std::stod(std::getenv("page_rank_dampening_factor"));
		}

		if (std::getenv("test_iterations") != nullptr) {
			tp.test_iterations = std::stol(std::getenv("test_iterations"));
		}

		if (std::getenv("pmem_directory") != nullptr) {
			tp.pmem_directory = std::string(std::getenv("pmem_directory"));
		}

		printf("Test Parameters:\n");
		printf("\talloc_size: %sB\n", FormatUtils::format_number(tp.alloc_size).c_str());
		printf("\tnum_vertices: %s\n", FormatUtils::format_number(tp.num_vertices).c_str());
		printf("\tmin_degree: %s\n", FormatUtils::format_number(tp.min_degree).c_str());
		printf("\tmax_degree: %s\n", FormatUtils::format_number(tp.max_degree).c_str());
		printf("\tmin_value: %s\n", FormatUtils::format_number(tp.min_value).c_str());
		printf("\tmax_value: %s\n", FormatUtils::format_number(tp.max_value).c_str());
		printf("\tpage_rank_iterations: %s\n", FormatUtils::format_number(tp.page_rank_iterations).c_str());
		printf("\tpage_rank_dampening_factor: %s\n", FormatUtils::format_number(tp.page_rank_dampening_factor).c_str());
		printf("\ttest_iterations: %s\n", FormatUtils::format_number(tp.test_iterations).c_str());
		printf("\tGraph Path: %s\n", tp.graph_path.c_str());
		printf("\tPMEM Directory: %s\n", tp.pmem_directory.c_str());

		return tp;
	}

	void benchmark_page_rank(const Benchmark::Parameters& tp) {

		BlockTimer timer("Page Rank Benchmark");
		printf("Page Rank Benchmark\n");

		std::string temp_graph_path = "./tmp/page_rank_graph.csv";

		std::vector<std::vector<double>> dram_metrics;
		std::vector<std::vector<double>> pmem_metrics;

		{
			printf("DRAM\n");
			GraphCRS graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs(tp.num_vertices, tp.min_degree, tp.max_degree, tp.min_value, tp.max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				graph.save(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load(tp.graph_path);
			}

			dram_metrics = run_page_rank(tp, graph);
		}

		{
			printf("PMEM\n");
			PMEM::GraphCRS graph_pmem(tp.pmem_directory);

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(temp_graph_path, tp.pmem_directory);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(tp.graph_path, tp.pmem_directory);
			}

			printf("Is pmem: %s\n", graph_pmem.is_pmem() ? "True" : "False");

			pmem_metrics = run_page_rank(tp, graph_pmem);
			graph_pmem.free();
		}

		printf("Time Elapsed (s)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[0], pmem_metrics[0]);

		printf("Edges per Second\n");
		BenchmarkUtils::compare_metrics(dram_metrics[1], pmem_metrics[1]);
	}

	std::vector<std::vector<double>> run_page_rank(const Benchmark::Parameters& tp, const Graph& graph) {

		BlockTimer timer("Page Rank");
		printf("Page Rank\n");

		std::vector<std::vector<double>> metrics(2);
		std::vector<double>& time_elapsed_v = metrics[0];
		std::vector<double>& edges_per_second_v = metrics[1];

		printf("Number of Vertices: %u\n", graph.num_vertices());
		printf("Number of Edges: %u\n", graph.num_edges());
		printf("Memory Size: %lu B\n", graph.byte_size());
		printf("Iteration, Time Elapsed (s), Edges per Second\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			graph.page_rank(tp.page_rank_iterations, tp.page_rank_dampening_factor);
			timer.end();

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			time_elapsed_v.push_back(time_elapsed_seconds);
			edges_per_second_v.push_back(graph.num_edges() / time_elapsed_seconds);
			printf("%u, %.3f, %.3f\n", iter, time_elapsed_seconds, edges_per_second_v.back());
		}

		BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);
		BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);

		return metrics;
	}

	void benchmark_breadth_first_traversal(const Benchmark::Parameters& tp) {

		BlockTimer timer("Breadth First Traversal Benchmark");
		printf("Breadth First Traversal Benchmark\n");

		std::string temp_graph_path = "./tmp/bfs_graph.csv";

		/* Create a list of vertices so the tests perform the same traversals */
		std::vector<uint32_t> start_vertices;

		std::vector<std::vector<double>> dram_metrics;
		std::vector<std::vector<double>> pmem_metrics;

		{
			printf("DRAM\n");
			GraphCRS graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs(tp.num_vertices, tp.min_degree, tp.max_degree, tp.min_value, tp.max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				graph.save(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load(tp.graph_path);
			}

			for (uint32_t i = 0; i < tp.test_iterations; i++) {
				start_vertices.push_back(graph.num_vertices() * double(i) / double(tp.test_iterations));
			}

			dram_metrics = run_breadth_first_traversal(tp, graph, start_vertices);
		}

		{
			printf("PMEM\n");
			PMEM::GraphCRS graph_pmem(tp.pmem_directory);

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(temp_graph_path, tp.pmem_directory);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(tp.graph_path, tp.pmem_directory);
			}

			printf("Is pmem: %s\n", graph_pmem.is_pmem() ? "True" : "False");

			pmem_metrics = run_breadth_first_traversal(tp, graph_pmem, start_vertices);
			graph_pmem.free();
		}

		printf("Time Elapsed (s)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[0], pmem_metrics[0]);

		printf("Edges per Second\n");
		BenchmarkUtils::compare_metrics(dram_metrics[1], pmem_metrics[1]);
	}

	std::vector<std::vector<double>> run_breadth_first_traversal(const Benchmark::Parameters& tp, const Graph& graph, std::vector<uint32_t> start_vertices) {

		BlockTimer timer("Breadth First Traversal");
		printf("Breadth First Traversal\n");
		std::vector<std::vector<double>> metrics(2);

		std::vector<double>& time_elapsed_v = metrics[0];
		std::vector<double>& edges_per_second_v = metrics[1];

		printf("Number of Vertices: %u\n", graph.num_vertices());
		printf("Number of Edges: %u\n", graph.num_edges());
		printf("Memory Size: %lu B\n", graph.byte_size());
		printf("Iteration, Time Elapsed (s), Edges per Second\n");
		for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
			Timer timer;
			graph.breadth_first_traversal(start_vertices[iter - 1]);
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

	std::vector<std::vector<double>> run_memory(const Benchmark::Parameters& tp, char* arr, const size_t size) {

		const size_t latency_loads = std::min(double(size / 20), 1e9);
		printf("Memory Size: %sB\n", FormatUtils::format_number(size).c_str());
		printf("Latency Loads: %sB\n", FormatUtils::format_number(latency_loads).c_str());

		std::vector<std::vector<double>> metric_v(4);

		BlockTimer timer("Memory Test");

		/* Read linear */
		{
			BlockTimer t_timer("Read Linear");
			printf("Read Linear\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (B/s)\n");

			std::vector<double>& read_linear_bandwidth = metric_v[0];

			int64_t sum = 1;
			const int64_t* test_mem = (int64_t*)arr;
			const size_t test_mem_size = size / sizeof(int64_t);

			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;
#pragma omp parallel for
				for (size_t i = 0; i < test_mem_size; i++) {
					sum *= test_mem[i];
				}
				timer.end();
				double time_elapsed = timer.get_time_elapsed() / 1e9;
				read_linear_bandwidth.push_back((double)size / time_elapsed);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed, read_linear_bandwidth.back());
			}
			printf("IGNORE(%ld)\n", sum);
			BenchmarkUtils::print_metrics("Bandwidth", read_linear_bandwidth);
		}

		/* Read Random */
		{
			BlockTimer t_timer("Read Random");
			printf("Read Random\n");
			printf("Iteration, Time Elapsed (s), Latency (ns)\n");

			std::vector<double>& read_random_latency = metric_v[1];

			std::default_random_engine generator;
			std::uniform_int_distribution<uint64_t> distribution(0, size);
			auto indexGen = std::bind(distribution, generator);

			char sum = 0;
			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;

				for (size_t i = 0; i < latency_loads; i++) {
					sum += arr[indexGen()];
				}

				timer.end();
				double time_elapsed = timer.get_time_elapsed();
				read_random_latency.push_back(time_elapsed / latency_loads);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed / 1e9, read_random_latency.back());

			}
			printf("IGNORE(%c)\n", sum);
			BenchmarkUtils::print_metrics("Latency", read_random_latency);
		}

		/* Write linear */
		{
			BlockTimer t_timer("Write Linear");
			printf("Write Linear\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (B/s)\n");

			std::vector<double>& write_linear_bandwidth = metric_v[2];

			uint64_t* test_mem = (uint64_t*)arr;
			const size_t test_mem_size = size / sizeof(uint64_t);

			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;
#pragma omp parallel for
				for (size_t i = 0; i < test_mem_size; i++) {
					test_mem[i] = 0;
				}
				timer.end();
				double time_elapsed = timer.get_time_elapsed() / 1e9;
				write_linear_bandwidth.push_back((double)size / time_elapsed);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed, write_linear_bandwidth.back());
			}
			BenchmarkUtils::print_metrics("Bandwidth", write_linear_bandwidth);
		}

		/* Write Random */
		{
			BlockTimer t_timer("Write Random");
			printf("Write Random\n");
			printf("Iteration, Time Elapsed (s), Latency (ns)\n");

			std::vector<double>& write_random_latency = metric_v[3];

			std::default_random_engine generator;
			std::uniform_int_distribution<uint64_t> distribution(0, size);
			auto indexGen = std::bind(distribution, generator);

			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;

				for (size_t i = 0; i < latency_loads; i++) {
					arr[indexGen()] = 0;
				}

				timer.end();
				double time_elapsed = timer.get_time_elapsed();
				write_random_latency.push_back(time_elapsed / latency_loads);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed / 1e9, write_random_latency.back());
			}
			BenchmarkUtils::print_metrics("Latency", write_random_latency);
		}

		return metric_v;
	}

	void benchmark_memory(const Benchmark::Parameters& tp) {

		BlockTimer timer("Memory Benchmark");
		printf("Memory Benchmark\n");

		printf("DRAM\n");
		char* dram_array = new char[tp.alloc_size];
		/* Touch the first and last element to make sure the OS allocated the memory */
		printf("IGNORE(%c)\n", dram_array[0] + dram_array[tp.alloc_size - 1]);
		std::vector<std::vector<double>> dram_metrics = run_memory(tp, dram_array, tp.alloc_size);
		delete dram_array;

		printf("Persistent Memory\n");
		PMEM::ptr pmem = PMEM::ptr(tp.pmem_directory, PMEM::FILE::TEMP, tp.alloc_size);
		char* pmem_array = pmem.as<char*>();

		printf("Is pmem: %s\n", pmem.is_pmem() ? "True" : "False");
		printf("Mapped length: %sB\n", FormatUtils::format_number(pmem.mapped_len()).c_str());

		if (pmem_array == nullptr) {
			printf("Trouble allocating persistent memory\n");
			return;
		}
		/* Touch the first and last element to make sure the OS allocated the memory */
		printf("IGNORE(%c)\n", pmem_array[0] + pmem_array[tp.alloc_size - 1]);
		std::vector<std::vector<double>> pmem_metrics = run_memory(tp, pmem_array, tp.alloc_size);
		pmem.free();

		printf("Read Linear Bandwith (B/s)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[0], pmem_metrics[0]);

		printf("Read Random Latency (ns)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[1], pmem_metrics[1]);

		printf("Write Linear Bandwith (B/s)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[2], pmem_metrics[2]);

		printf("Write Random Latency (ns)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[3], pmem_metrics[3]);
	}

	std::string get_graph_name(const std::string& graph_path) {
		size_t start_index = graph_path.find_last_of("/") + 1;
		size_t end_index = graph_path.find_last_of(".") - start_index;
		return graph_path.substr(start_index, end_index);
	}
}