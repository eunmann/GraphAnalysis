#include <stdio.h>

#include "GraphUtils.hpp"
#include "PMEM/Tests.hpp"
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include <inttypes.h>
#include "PMEM/ptr.hpp"
#include <random>
#include <string>

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

	PMEM::GraphCRS g = GraphUtils::create_graph_crs_pmem("/tmp/", num_vertices, min_degree, max_degree, min_value, max_value);
	g.print();
	g.save("./tmp/graph_test.csv");
	g.free();
}

void memory_test(char* arr, const uint64_t size) {

	std::default_random_engine generator;
	std::uniform_int_distribution<uint64_t> distribution(0, size);
	auto indexGen = std::bind(distribution, generator);
	uint64_t sum = 0;

	BlockTimer timer("Memory Test");

	/* Read straight through */
	{
		BlockTimer rs_timer("Read Straight");
		for (uint64_t i = 0; i < size; i++) {
			sum += arr[i];
		}
		printf("Sum: %lu\n", sum);
	}

	/* Read Random */
	{
		BlockTimer rr_timer("Read Random");
		sum = 0;
		for (uint64_t i = 0; i < size; i++) {
			sum += arr[indexGen()];
		}
		printf("Sum: %lu\n", sum);
	}

	/* Write straight */
	{
		BlockTimer ws_timer("Write Straight");
		for (uint64_t i = 0; i < size; i++) {
			arr[i] = 0;
		}
	}

	/* Write Random */
	{
		BlockTimer wr_timer("Write Random");
		for (uint64_t i = 0; i < size; i++) {
			arr[indexGen()] = 0;
		}
	}
}

void pmem_vs_dram_test(const uint64_t alloc_size) {
	printf("Allocation Size: %lu\n", alloc_size);

	{
		printf("DRAM\n");
		char* array = new char[alloc_size];
		memory_test(array, alloc_size);
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
		memory_test(array, alloc_size);
		pmem.free();
	}
}

int main(int argc, char** argv) {
	Timer timer("Time Elapsed");
	printf("Graph Algorithm Performance Analysis on Persistent Memory Machines\n");
	printf("by Evan Unmann\n");

	size_t alloc_size = 1 * 1e9;

	if (argc > 1) {
		alloc_size = std::stol(std::string(argv[1]));
	}

	graph_test();

	timer.end();
	timer.print();

	return 0;
}