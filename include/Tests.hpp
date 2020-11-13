typedef unsigned long size_t;

namespace Tests {
	void PMEM_tests();

	void graph_test();

	void memory_benchmark(char* arr, const size_t size);

	void pmem_vs_dram_benchmark(const size_t alloc_size);
}