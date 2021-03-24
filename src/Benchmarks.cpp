#include <stdio.h>
#include <functional>
#include <immintrin.h>
#include <omp.h>
#include <limits>
#include <random>

#include "GraphUtils.hpp"
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include "FormatUtils.hpp"
#include "BenchmarkUtils.hpp"
#include "GNUPlot/Plot.hpp"
#include "Benchmarks.hpp"
#include "GraphAlgorithms.hpp"
#include "PMEM/allocator.hpp"
#include "Stream.hpp"

namespace Benchmark {

	Benchmark::Parameters get_parameters() {
		Parameters tp;

		if (std::getenv("mem_alloc_size") != nullptr) {
			tp.mem_alloc_size = std::stol(std::getenv("mem_alloc_size"));
		}

		if (std::getenv("graph_num_vertices") != nullptr) {
			tp.graph_num_vertices = std::stol(std::getenv("graph_num_vertices"));
		}

		if (std::getenv("graph_min_degree") != nullptr) {
			tp.graph_min_degree = std::stol(std::getenv("graph_min_degree"));
		}

		if (std::getenv("graph_max_degree") != nullptr) {
			tp.graph_max_degree = std::stol(std::getenv("graph_max_degree"));
		}

		if (std::getenv("graph_min_value") != nullptr) {
			tp.graph_min_value = std::stol(std::getenv("graph_min_value"));
		}

		if (std::getenv("graph_max_value") != nullptr) {
			tp.graph_max_value = std::stol(std::getenv("graph_max_value"));
		}

		if (std::getenv("page_rank_iterations") != nullptr) {
			tp.page_rank_iterations = std::stol(std::getenv("page_rank_iterations"));
		}

		if (std::getenv("page_rank_num_dampening_factors") != nullptr) {
			tp.page_rank_num_dampening_factors = std::stol(std::getenv("page_rank_num_dampening_factors"));
		}

		if (std::getenv("test_iterations") != nullptr) {
			tp.test_iterations = std::stol(std::getenv("test_iterations"));
		}

		if (std::getenv("out_dir") != nullptr) {
			tp.out_dir = std::string(std::getenv("out_dir"));
		}

		printf("Test Parameters:\n");
		printf("\tmem_alloc_size: %sB\n", FormatUtils::format_number(tp.mem_alloc_size).c_str());
		printf("\tgraph_num_vertices: %s\n", FormatUtils::format_number(tp.graph_num_vertices).c_str());
		printf("\tgraph_min_degree: %s\n", FormatUtils::format_number(tp.graph_min_degree).c_str());
		printf("\tgraph_max_degree: %s\n", FormatUtils::format_number(tp.graph_max_degree).c_str());
		printf("\tgraph_min_value: %s\n", FormatUtils::format_number(tp.graph_min_value).c_str());
		printf("\tgraph_max_value: %s\n", FormatUtils::format_number(tp.graph_max_value).c_str());
		printf("\tpage_rank_iterations: %s\n", FormatUtils::format_number(tp.page_rank_iterations).c_str());
		printf("\tpage_rank_num_dampening_factors: %s\n", FormatUtils::format_number(tp.page_rank_num_dampening_factors).c_str());
		printf("\ttest_iterations: %s\n", FormatUtils::format_number(tp.test_iterations).c_str());
		printf("\tGraph Path: %s\n", tp.graph_path.c_str());
		printf("\tOut Dir Path: %s\n", tp.out_dir.c_str());

		return tp;
	}

	void benchmark_page_rank(const Benchmark::Parameters& tp) {

		BlockTimer timer("Page Rank Benchmark");
		printf("Page Rank Benchmark\n");

		std::string temp_graph_path = "./tmp/page_rank_graph.csv";

		std::vector<double> metrics_DD;
		std::vector<double> metrics_DP;
		std::vector<double> metrics_PD;
		std::vector<double> metrics_PP;

		{
			GraphCRS<std::allocator> graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs<std::allocator>(tp.graph_num_vertices, tp.graph_min_degree, tp.graph_max_degree, tp.graph_min_value, tp.graph_max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				GraphUtils::save(graph, temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load<std::allocator>(tp.graph_path);
			}

			metrics_DD = run_page_rank<std::allocator, std::allocator>(tp, graph);
			metrics_DP = run_page_rank<std::allocator, PMEM::allocator>(tp, graph);
			graph.free();
		}

		{
			GraphCRS<PMEM::allocator> graph;

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph = GraphUtils::load<PMEM::allocator>(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load<PMEM::allocator>(tp.graph_path);
			}

			metrics_PD = run_page_rank<PMEM::allocator, std::allocator>(tp, graph);
			metrics_PP = run_page_rank<PMEM::allocator, PMEM::allocator>(tp, graph);
			graph.free();
		}

		std::string csv_path = tp.out_dir;
		csv_path += "/page_rank_metrics.csv";

		BenchmarkUtils::create_csv(csv_path, { "graph", "DD", "DP", "PD", "PP" });
		BenchmarkUtils::save_graph_metrics_csv(csv_path, tp.graph_name, { metrics_DD, metrics_DP, metrics_PD, metrics_PP });
	}

	void benchmark_page_rank_sizes(const Benchmark::Parameters& tp) {
		BlockTimer timer("Page Rank Benchmark");
		printf("Page Rank Benchmark\n");

		std::string temp_graph_path = "./tmp/page_rank_graph.csv";

		std::vector<std::vector<double>> metrics_DD;
		std::vector<std::vector<double>> metrics_DP;
		std::vector<std::vector<double>> metrics_PD;
		std::vector<std::vector<double>> metrics_PP;

		std::vector<std::string> csv_headers = { "comb" };
		for (auto& num_page_ranks : tp.page_rank_dampening_factors) {
			csv_headers.push_back(std::to_string(num_page_ranks));
		}
		std::string csv_path = tp.out_dir;
		csv_path += "/page_rank_metrics_";
		csv_path += tp.graph_name;
		csv_path += ".csv";
		BenchmarkUtils::create_csv(csv_path, csv_headers);

		{
			GraphCRS<std::allocator> graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs<std::allocator>(tp.graph_num_vertices, tp.graph_min_degree, tp.graph_max_degree, tp.graph_min_value, tp.graph_max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				GraphUtils::save(graph, temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load<std::allocator>(tp.graph_path);
			}

			for (auto& num_page_ranks : tp.page_rank_dampening_factors) {
				Benchmark::Parameters pr_tp = tp;
				pr_tp.page_rank_num_dampening_factors = num_page_ranks;
				metrics_DD.push_back(run_page_rank<std::allocator, std::allocator>(pr_tp, graph));
			}

			BenchmarkUtils::save_graph_metrics_csv(csv_path, "DD", metrics_DD);

			for (auto& num_page_ranks : tp.page_rank_dampening_factors) {
				Benchmark::Parameters pr_tp = tp;
				pr_tp.page_rank_num_dampening_factors = num_page_ranks;
				metrics_DP.push_back(run_page_rank<std::allocator, PMEM::allocator>(tp, graph));
			}

			BenchmarkUtils::save_graph_metrics_csv(csv_path, "DP", metrics_DP);

			graph.free();
		}

		{
			GraphCRS<PMEM::allocator> graph;

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph = GraphUtils::load<PMEM::allocator>(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load<PMEM::allocator>(tp.graph_path);
			}

			for (auto& num_page_ranks : tp.page_rank_dampening_factors) {
				Benchmark::Parameters pr_tp = tp;
				pr_tp.page_rank_num_dampening_factors = num_page_ranks;
				metrics_PD.push_back(run_page_rank<PMEM::allocator, std::allocator>(tp, graph));
			}

			BenchmarkUtils::save_graph_metrics_csv(csv_path, "PD", metrics_PD);


			for (auto& num_page_ranks : tp.page_rank_dampening_factors) {
				Benchmark::Parameters pr_tp = tp;
				pr_tp.page_rank_num_dampening_factors = num_page_ranks;
				metrics_PP.push_back(run_page_rank<PMEM::allocator, PMEM::allocator>(tp, graph));
			}

			BenchmarkUtils::save_graph_metrics_csv(csv_path, "PP", metrics_PP);


			graph.free();
		}
	}

	void benchmark_breadth_first_traversal(const Benchmark::Parameters& tp) {

		BlockTimer timer("Breadth First Traversal Benchmark");
		printf("Breadth First Traversal Benchmark\n");

		std::string temp_graph_path = "./tmp/bfs_graph.csv";

		/* Create a list of vertices so the tests perform the same traversals */
		std::vector<uint32_t> start_vertices;

		std::vector<double> metrics_DD;
		std::vector<double> metrics_DP;
		std::vector<double> metrics_PD;
		std::vector<double> metrics_PP;

		{
			GraphCRS<std::allocator> graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs<std::allocator>(tp.graph_num_vertices, tp.graph_min_degree, tp.graph_max_degree, tp.graph_min_value, tp.graph_max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				GraphUtils::save(graph, temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load<std::allocator>(tp.graph_path);
			}

			for (uint32_t i = 0; i < tp.test_iterations; i++) {
				start_vertices.push_back(graph.num_vertices() * double(i) / double(tp.test_iterations));
			}

			metrics_DD = run_breadth_first_traversal<std::allocator, std::allocator>(tp, graph, start_vertices);
			metrics_DP = run_breadth_first_traversal<std::allocator, PMEM::allocator>(tp, graph, start_vertices);
			graph.free();
		}

		{
			GraphCRS<PMEM::allocator> graph;

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph = GraphUtils::load<PMEM::allocator>(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load<PMEM::allocator>(tp.graph_path);
			}

			metrics_PD = run_breadth_first_traversal<PMEM::allocator, std::allocator>(tp, graph, start_vertices);
			metrics_PP = run_breadth_first_traversal<PMEM::allocator, PMEM::allocator>(tp, graph, start_vertices);
			graph.free();
		}

		std::string csv_path = tp.out_dir;
		csv_path += "/bfs_metrics.csv";
		BenchmarkUtils::create_csv(csv_path, { "graph", "DD", "DP", "PD", "PP" });
		BenchmarkUtils::save_graph_metrics_csv(csv_path, tp.graph_name, { metrics_DD, metrics_DP, metrics_PD, metrics_PP });
	}


	std::vector<std::vector<double>> run_memory(const Benchmark::Parameters& tp, char* arr, size_t size) {

		const size_t latency_loads = 25e6;
		printf("Memory Size: %sB\n", FormatUtils::format_number(size).c_str());
		printf("Latency Loads: %sB\n", FormatUtils::format_number(latency_loads).c_str());

		std::vector<std::vector<double>> metric_v(3);

		BlockTimer timer("Memory Test");

		/* Read Sequential */
		{
			BlockTimer t_timer("Read Sequential");
			printf("Read Sequential\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (B/s)\n");

			std::vector<double>& read_linear_bandwidth = metric_v[0];
			__m256i sum[omp_get_max_threads()];

			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;

				for (auto& v : sum) {
					v = _mm256_setzero_si256();
				}
				__m256i* test_mem = reinterpret_cast<__m256i*>(arr);
				const size_t test_mem_size = size / sizeof(__m256i);

#pragma omp parallel
				{
					__m256i s = sum[omp_get_thread_num()];
#pragma omp for schedule(static)
					for (size_t i = 0; i < test_mem_size; i++) {
						s += _mm256_load_si256(test_mem + i);
					}

					sum[omp_get_thread_num()] = s;
				}

				timer.end();
				double time_elapsed = timer.get_time_elapsed() / 1e9;
				read_linear_bandwidth.push_back((double)size / time_elapsed);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed, read_linear_bandwidth.back());
			}

			printf("IGNORE(%llu)\n", _mm256_extract_epi64(sum[0], 0));
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
			auto index_gen = std::bind(distribution, generator);

			char sum = 0;
			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;

				char v = arr[0];
				for (size_t i = 0; i < latency_loads; i++) {
					v = arr[index_gen() + v];
					sum += v;
				}

				timer.end();
				double time_elapsed = timer.get_time_elapsed();
				read_random_latency.push_back(time_elapsed / latency_loads);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed / 1e9, read_random_latency.back());
			}

			printf("IGNORE(%c)\n", sum);
			BenchmarkUtils::print_metrics("Latency", read_random_latency);
		}

		/* Write Sequential */
		{
			BlockTimer t_timer("Write Sequential");
			printf("Write Sequential\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (B/s)\n");

			std::vector<double>& write_linear_bandwidth = metric_v[2];

			for (uint32_t iter = 1; iter <= tp.test_iterations; iter++) {
				Timer timer;

				__m256i* test_mem = reinterpret_cast<__m256i*>(arr);
				const size_t test_mem_size = size / sizeof(__m256i);

#pragma omp parallel for schedule(static)
				for (size_t i = 0; i < test_mem_size; i++) {
					_mm256_store_si256(test_mem + i, _mm256_setzero_si256());
				}

				timer.end();
				double time_elapsed = timer.get_time_elapsed() / 1e9;
				write_linear_bandwidth.push_back((double)size / time_elapsed);
				printf("%u, %.3f, %.3f\n", iter, time_elapsed, write_linear_bandwidth.back());
			}

			BenchmarkUtils::print_metrics("Bandwidth", write_linear_bandwidth);
		}

		return metric_v;
	}

	void benchmark_memory(const Benchmark::Parameters& tp) {

		BlockTimer timer("Memory Benchmark");
		printf("Memory Benchmark\n");

		printf("DRAM\n");

		const size_t align_bytes = 16;
		const size_t align_alloc_size = tp.mem_alloc_size + align_bytes;
		char* dram_array = new char[align_alloc_size];
		char* dram_array_aligned = dram_array + ((uintptr_t)dram_array % align_bytes);

		BenchmarkUtils::set_zeros(dram_array_aligned, tp.mem_alloc_size);
		std::vector<std::vector<double>> dram_metrics = run_memory(tp, dram_array_aligned, tp.mem_alloc_size);
		delete dram_array;

		printf("Persistent Memory\n");
		PMEM::allocator<char> pmem_alloc;
		char* pmem_array = pmem_alloc.allocate(align_alloc_size);
		printf("Is pmem: %s\n", pmem_alloc.is_pmem() ? "True" : "False");

		if (pmem_array == nullptr) {
			printf("Trouble allocating persistent memory\n");
			return;
		}

		char* pmem_array_aligned = pmem_array + ((uintptr_t)pmem_array % align_bytes);

		BenchmarkUtils::set_zeros(pmem_array_aligned, tp.mem_alloc_size);
		std::vector<std::vector<double>> pmem_metrics = run_memory(tp, pmem_array_aligned, tp.mem_alloc_size);
		pmem_alloc.deallocate(pmem_array, align_alloc_size);

		printf("Read Sequential Bandwith (B/s)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[0], pmem_metrics[0]);

		printf("Read Random Latency (ns)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[1], pmem_metrics[1]);

		printf("Write Sequential Bandwith (B/s)\n");
		BenchmarkUtils::compare_metrics(dram_metrics[2], pmem_metrics[2]);

		std::string csv_path = tp.out_dir;
		csv_path += "/mem_metrics.csv";
		BenchmarkUtils::create_csv(csv_path, { "benchmark", "dram", "pmem" });
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "Read Sequential Bandwidth", "B/s", dram_metrics[0], pmem_metrics[0]);
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "Read Random Latency", "ns", dram_metrics[1], pmem_metrics[1]);
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "Write Sequential Bandwidth", "B/s", dram_metrics[2], pmem_metrics[2]);
	}

	void benchmark_STREAM(const Benchmark::Parameters& tp) {

		BlockTimer timer("STREAM Benchmark");
		printf("STREAM Benchmark\n");
		printf("DRAM\n");
		std::vector<double> dram_metrics = STREAM::run(false);
		printf("PMEM\n");
		std::vector<double> pmem_metrics = STREAM::run(true);

		std::string csv_path = tp.out_dir;
		csv_path += "/mem_metrics.csv";
		BenchmarkUtils::create_csv(csv_path, { "benchmark", "dram", "pmem" });
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "STREAM Copy", "B/s", dram_metrics[0], pmem_metrics[0]);
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "STREAM Scale", "B/s", dram_metrics[1], pmem_metrics[1]);
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "STREAM Add", "B/s", dram_metrics[2], pmem_metrics[2]);
		BenchmarkUtils::save_mem_metrics_csv(csv_path, "STREAM Triad", "B/s", dram_metrics[3], pmem_metrics[3]);
	}
}