#include "PMEM/Tests.hpp"
#include <stdio.h>
#include "GraphUtils.hpp"
#include <random>
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include "FormatUtils.hpp"
#include "BenchmarkUtils.hpp"

namespace Tests {
	void PMEM_tests() {
		printf("First test, simple struct write and read.\n");
		PMEM::Tests::libpmemobj_example_write_1();
		PMEM::Tests::libpmemobj_example_read_1();

		printf("Second test, simple struct write and read with type safety.\n");
		PMEM::Tests::libpmemobj_example_write_2();
		PMEM::Tests::libpmemobj_example_read_2();

		printf("Third test, this is using an API that I made.\n");
		PMEM::Tests::pmem_as_volatile_API();
	}

	void graph_test() {
		const uint32_t num_vertices = 4;
		const uint32_t min_degree = 1;
		const uint32_t max_degree = 2;
		const float min_value = 1;
		const float max_value = 2;

		std::string graph_file_path = "./graph_examples/test_format.csv";
		std::string simple_graph_path = "./graph_examples/simple_graph.csv";

		printf("First Graph\n");
		PMEM::GraphCRS g1 = GraphUtils::create_graph_crs_pmem("/tmp/", num_vertices, min_degree, max_degree, min_value, max_value);
		g1.print();
		g1.save(graph_file_path);
		g1.free();

		printf("Second Graph\n");
		PMEM::GraphCRS g2 = GraphUtils::load_as_pmem(simple_graph_path, "/tmp/");
		g2.print();
		std::vector<uint32_t> shortest_path = g2.shortest_path(2, 4);

		if (shortest_path.size() > 0 && shortest_path.size() <= 5) {

			printf("[");
			for (int i = 0; i < shortest_path.size() - 1; i++) {
				printf(" %u ->", shortest_path[i]);
			}
			printf(" %u ]\n", shortest_path.back());
		}
		else {
			printf("Shortest Path has wrong size [%lu]\n", shortest_path.size());
		}
		g2.free();
	}

	void graph_test_page_rank(const uint32_t num_vertices) {
		const uint32_t min_degree = 1;
		const uint32_t max_degree = 10;
		const float min_value = 1;
		const float max_value = 5;
		const uint32_t iterations = 100;
		const float dampening_factor = 0.8;
		const uint32_t test_iterations = 10;

		printf("Testing Page Rank\n");
		printf("\tNumber of Vertices: %u\n", num_vertices);
		printf("\tMinimum Degree: %u\n", min_degree);
		printf("\tMaximum Degree: %u\n", max_degree);
		printf("\tIterations: %u\n", iterations);
		printf("\tDampening Factor: %f\n", dampening_factor);
		printf("\tTest Iterations: %u\n", test_iterations);

		std::string temp_graph_path = "./tmp/page_rank_graph.csv";

		{
			printf("Graph DRAM\n");
			GraphCRS graph = GraphUtils::create_graph_crs(num_vertices, min_degree, max_degree, min_value, max_value);
			printf("Number of Edges: %u\n", graph.num_edges());
			graph.save(temp_graph_path);
			std::vector<double> time_elapsed_v;
			printf("Iteration, Time Elapsed (s),Edges per Second\n");
			for (uint32_t i = 1; i <= test_iterations; i++) {
				Timer timer;
				graph.page_rank(iterations, dampening_factor);
				timer.end();

				double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
				time_elapsed_v.push_back(time_elapsed_seconds);
				printf("%d,%.3f,%.3f\n", i, time_elapsed_seconds, graph.num_edges() / time_elapsed_seconds);
			}

			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double> edges_per_second_v;
			for (auto& t : time_elapsed_v) {
				edges_per_second_v.push_back(graph.num_edges() / t);
			}
			BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);
		}

		printf("Graph PMEM\n");
		PMEM::GraphCRS graph_pmem = GraphUtils::load_as_pmem(temp_graph_path, "/pmem/");
		printf("\tNumber of Edges: %u\n", graph_pmem.num_edges());
		std::vector<double> time_elapsed_v;
		printf("Iteration, Time Elapsed (s),Edges per Second\n");
		for (uint32_t i = 1; i <= test_iterations; i++)
		{
			Timer timer("Page Rank PMEM");
			graph_pmem.page_rank(iterations, dampening_factor);
			timer.end();

			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			time_elapsed_v.push_back(time_elapsed_seconds);
			printf("%d,%.3f,%.3f\n", i, time_elapsed_seconds, graph_pmem.num_edges() / time_elapsed_seconds);
		}
		BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

		std::vector<double> edges_per_second_v;
		for (auto& t : time_elapsed_v) {
			edges_per_second_v.push_back(graph_pmem.num_edges() / t);
		}
		BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);
		graph_pmem.free();
	}

	void memory_benchmark(char* arr, const size_t size) {

		int iter_per_test = 10;
		const size_t latency_loads = size / 1000;
		printf("Memory Benchmark\n");
		printf("Memory Size: %sB\n", FormatUtils::format_number(size).c_str());
		printf("Latency Loads: %sB\n", FormatUtils::format_number(latency_loads).c_str());
		printf("Iterations per Test: %d\n", iter_per_test);

		double size_gigabytes = size / 1e9;
		auto print_bandwidth = [&size_gigabytes](const int iteration, const Timer& timer) {
			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			printf("%d,%.3f,%.3f\n", iteration, time_elapsed_seconds, size_gigabytes / time_elapsed_seconds);
		};

		auto print_latency = [&latency_loads](const int iteration, const Timer& timer) {
			printf("%d,%.3f,%.3f\n", iteration, timer.get_time_elapsed() / 1e9, 1.0 * timer.get_time_elapsed() / latency_loads);
		};

		auto print_vec_bandwith = [&size_gigabytes](const std::vector<double> time_elapsed_v) {
			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double> bandwidth_v;
			for (auto& t : time_elapsed_v) {
				bandwidth_v.push_back(size_gigabytes / t);
			}
			BenchmarkUtils::print_metrics("Bandwidth", bandwidth_v);
		};

		auto print_vec_latency = [&latency_loads](const std::vector<double> time_elapsed_v) {
			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double> latency_v;
			for (auto& t : time_elapsed_v) {
				latency_v.push_back(1e9 * t / latency_loads);
			}
			BenchmarkUtils::print_metrics("Latency", latency_v);
		};

		uint64_t sum = 0;
		/* Convert the byte pointer to a larger unit so loads pull as much as possible from memory */
		uint64_t* test_mem = (uint64_t*)arr;
		const size_t test_mem_size = size / sizeof(uint64_t);

		BlockTimer timer("Memory Test");

		/* Read linear */
		{
			printf("Read Linear\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (GB/s)\n");
			std::vector<double> time_elapsed_v;
			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;
#pragma omp parallel for reduction(+:sum)
				for (uint64_t i = 0; i < test_mem_size; i++) {
					sum += test_mem[i];
				}
				timer.end();
				print_bandwidth(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}
			print_vec_bandwith(time_elapsed_v);
			printf("IGNORE(%lu)\n", sum);
		}

		/* Read Random */
		{
			printf("Read Random\n");
			printf("Iteration, Time Elapsed (s), Latency (ns)\n");
			std::vector<double> time_elapsed_v;
			std::default_random_engine generator;
			std::uniform_int_distribution<uint64_t> distribution(0, size);
			auto indexGen = std::bind(distribution, generator);

			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;
				sum = 0;

				for (uint64_t i = 0; i < latency_loads; i++) {
					sum += arr[indexGen()];
				}

				timer.end();
				print_latency(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}
			print_vec_latency(time_elapsed_v);
			printf("IGNORE(%lu)\n", sum);
		}

		/* Write linear */
		{
			printf("Write Linear\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (GB/s)\n");
			std::vector<double> time_elapsed_v;
			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;
#pragma omp parallel for
				for (uint64_t i = 0; i < test_mem_size; i++) {
					test_mem[i] = 0;
				}
				timer.end();
				print_bandwidth(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}
			print_vec_bandwith(time_elapsed_v);
		}

		/* Write Random */
		{
			printf("Write Random\n");
			printf("Iteration, Time Elapsed (s), Latency (ns)\n");

			std::default_random_engine generator;
			std::uniform_int_distribution<uint64_t> distribution(0, size);
			auto indexGen = std::bind(distribution, generator);

			std::vector<double> time_elapsed_v;
			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;

				for (uint64_t i = 0; i < latency_loads; i++) {
					arr[indexGen()] = 0;
				}

				timer.end();
				print_latency(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}

			print_vec_latency(time_elapsed_v);
		}
		printf("IGNORE(%lu)\n", sum);
	}

	void pmem_vs_dram_benchmark(const size_t alloc_size) {
		printf("Allocation Size: %sB\n", FormatUtils::format_number(alloc_size).c_str());

		{
			printf("DRAM\n");
			char* array = new char[alloc_size];
			memory_benchmark(array, alloc_size);
			delete array;
		}

		{
			printf("Persistent Memory\n");
			PMEM::ptr pmem = PMEM::ptr("/pmem/", PMEM::FILE::TEMP, alloc_size);
			char* array = pmem.as<char*>();

			printf("Is persistent: %s\n", pmem.is_persistent() ? "True" : "False");
			printf("Mapped length: %sB\n", FormatUtils::format_number(pmem.mapped_len()).c_str());

			if (array == nullptr) {
				printf("Trouble allocating persistent memory\n");
				return;
			}
			memory_benchmark(array, alloc_size);
			pmem.free();
		}
	}
}