#include "PMEM/Tests.hpp"
#include <stdio.h>
#include "GraphUtils.hpp"
#include <random>
#include "Timer.hpp"
#include "BlockTimer.hpp"

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

	void memory_benchmark(char* arr, const size_t size) {

		/*
			TODO(EMU):
			Also do a measurement for latency
		*/
		printf("Memory Benchmark\n");
		printf("Memory Size: %lu\n", size);

		BlockTimer timer("Memory Test");
		uint64_t sum = 0;
		auto print_stat = [&size](const Timer& timer) {
			double size_gigabytes = size / 1e9;
			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			printf("\tTime Elapsed: %.3f s", time_elapsed_seconds);
			printf("\tBandwidth: %.3f GB/s\n", size_gigabytes / time_elapsed_seconds);
		};

		/* Read linear */
		{
			Timer timer;
#pragma omp parallel for reduction(+:sum)
			for (uint64_t i = 0; i < size; i++) {
				sum += arr[i];
			}
			timer.end();
			printf("Read Linear\n");
			print_stat(timer);
		}

		/* Read Random */
		{
			Timer timer;
			sum = 0;

#pragma omp parallel
			{
				std::default_random_engine generator;
				std::uniform_int_distribution<uint64_t> distribution(0, size);
				auto indexGen = std::bind(distribution, generator);

#pragma omp for reduction(+:sum)
				for (uint64_t i = 0; i < size; i++) {
					sum += arr[indexGen()];
				}
			}
			timer.end();
			printf("Read Random\n");
			print_stat(timer);
		}

		/* Write linear */
		{
			Timer timer;
#pragma omp parallel for
			for (uint64_t i = 0; i < size; i++) {
				arr[i] = 0;
			}
			timer.end();
			printf("Write Linear\n");
			print_stat(timer);
		}

		/* Write Random */
		{
			Timer timer;
#pragma omp parallel
			{
				std::default_random_engine generator;
				std::uniform_int_distribution<uint64_t> distribution(0, size);
				auto indexGen = std::bind(distribution, generator);

#pragma omp for
				for (uint64_t i = 0; i < size; i++) {
					arr[indexGen()] = 0;
				}
			}
			timer.end();
			printf("Write Random\n");
			print_stat(timer);
		}

		printf("Ignore Value(needed so compiler doesn't optimize away benchmark code): %lu\n", sum);
	}

	void pmem_vs_dram_benchmark(const size_t alloc_size) {
		printf("Allocation Size: %lu\n", alloc_size);

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
			printf("Mapped length: %lu\n", pmem.mapped_len());

			if (array == nullptr) {
				printf("Trouble allocating persistent memory\n");
				return;
			}
			memory_benchmark(array, alloc_size);
			pmem.free();
		}
	}
}