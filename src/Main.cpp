#include <stdio.h>

#include "Graph.hpp"
#include "PMEMTest.hpp"
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include <inttypes.h>
#include "MemPool.hpp"

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

int main(int argc, char** argv) {
	Timer timer("Time Elapsed");
	printf("Graph Analysis for a Graph Algorithm on Persistent Memory Machines\n");
	printf("by Evan Unmann\n");

	PMEMTests();

	GraphTest();

	timer.end();
	timer.print();

	return 0;
}