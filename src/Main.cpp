#include <stdio.h>

#include "Graph.hpp"
#include "PMEMTest.hpp"
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include <inttypes.h>
#include "MemPool.hpp"
#include "Mem.hpp"
#include <random>

void PMEMTests() {
	printf("First test, simple struct write and read.\n");
	PMEMTest::simpleStructWrite();
	PMEMTest::simpleStructRead();

	printf("Second test, simple struct write and read with type safety.\n");
	PMEMTest::simpleStructWrite2();
	PMEMTest::simpleStructRead2();

	printf("Third test, using persistent memory like volatile memory.\n");
	PMEMTest::persistentMemoryAsVolatile();

	printf("Fourth test, this is using an API that I made.\n");
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
		Mem::MemPool memPool = Mem::MemPool(alloc_size);
		uint64_t* arr2 = memPool.malloc<uint64_t>(numberOfNodes * numberOfNodes);

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

		memPool.free_ptr(arr2);
		/* This function is actually called on destruction, so this is not required */
		memPool.free_pool();
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

	printf("Persistent Memory Available: %s\n", Mem::persistent_memory_available() ? "true" : "false");

	const size_t alloc_size = 500 * 1e6;
	printf("Allocation Size: %lu\n", alloc_size);

	{
		printf("DRAM\n");
		char* array = new char[alloc_size];
		memoryTest(array, alloc_size);
		delete array;
	}

	{
		printf("Persistent Memory\n");
		Mem::MemPool memPool = Mem::MemPool(alloc_size);
		char* array = Mem::p_new<char>(alloc_size);

		printf("Memory Kind: %s\n", memkind_detect_kind(array) == MEMKIND_DAX_KMEM ? "Persistent" : "DRAM");
		if (array == nullptr) {
			printf("Trouble allocating persistent memory\n");
			return 0;
		}
		memoryTest(array, alloc_size);
		memPool.free_ptr(array);
	}

	timer.end();
	timer.print();

	return 0;
}