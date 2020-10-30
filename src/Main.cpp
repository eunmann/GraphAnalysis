#include <stdio.h>

#include "Graph.hpp"
#include "PMEMTest.hpp"
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include <inttypes.h>
#include "PMEM.hpp"
#include <random>
#include <string>

void PMEMTests() {
	printf("First test, simple struct write and read.\n");
	PMEMTest::libpmemobj_example_write_1();
	PMEMTest::libpmemobj_example_read_1();

	printf("Second test, simple struct write and read with type safety.\n");
	PMEMTest::libpmemobj_example_write_2();
	PMEMTest::libpmemobj_example_read_2();

	printf("Third test, this is using an API that I made.\n");
	PMEMTest::persistentMemoryAsVolatileAPI();
}

void GraphTest() {
	const uint32_t numberOfNodes = 1 << 12;
	{
		BlockTimer timer("Graph using DRAM");
		uint64_t* arr = new uint64_t[numberOfNodes * numberOfNodes];

		Graph<uint64_t> g(numberOfNodes, arr);

		uint64_t t = 0;
		g.forEach([&](uint64_t& v, const uint32_t i, const uint32_t j) {
			v = 0;
			t += i + j;
			});

		printf("t: %ld\n", t);
		delete arr;
	}

	{
		BlockTimer timer("Graph using Persistent Memory");
		const size_t alloc_size = sizeof(uint64_t) * numberOfNodes * numberOfNodes;
		Mem::PMEM pmem = Mem::PMEM(alloc_size);
		uint64_t* arr2 = pmem.as<uint64_t*>();

		if (arr2 == nullptr) {
			printf("Error allocating memory for graph in persistent memory\n");
			return;
		}

		Graph<uint64_t> g2(numberOfNodes, arr2);
		uint64_t t = 0;
		g2.forEach([&](uint64_t& v, const uint32_t i, const uint32_t j) {
			v = 0;
			t += i + j;
			});

		printf("t: %ld\n", t);

		/* This function is actually called on destruction, so this is not required */
		pmem.free();
	}
}

void memoryTest(char* arr, const uint64_t size) {

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

int main(int argc, char** argv) {
	Timer timer("Time Elapsed");
	printf("Graph Analysis for a Graph Algorithm on Persistent Memory Machines\n");
	printf("by Evan Unmann\n");

	size_t alloc_size = 1 * 1e9;

	if (argc > 1) {
		alloc_size = std::stol(std::string(argv[1]));
	}

	printf("Allocation Size: %lu\n", alloc_size);

	{
		printf("DRAM\n");
		char* array = new char[alloc_size];
		memoryTest(array, alloc_size);
		delete array;
	}

	{
		printf("Persistent Memory\n");
		Mem::PMEM pmem = Mem::PMEM(alloc_size);
		char* array = pmem.as<char*>();

		printf("Is persistent: %s\n", pmem.is_persistent() ? "True" : "False");
		printf("Mapped length: %lu\n", pmem.mapped_length());

		if (array == nullptr) {
			printf("Trouble allocating persistent memory\n");
			return 0;
		}
		memoryTest(array, alloc_size);
		pmem.free();
	}

	timer.end();
	timer.print();

	return 0;
}