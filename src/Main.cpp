#include <stdio.h>
#include "Benchmarks.hpp"
#include "BlockTimer.hpp"
#include <string>
#include <vector>
#include <omp.h>
#include <exception>
#include <libpmem.h>
#include "PMEM/allocator.hpp"
#include "BenchmarkUtils.hpp"
#include "GraphUtils.hpp"

void print_info() {

	printf("Performance Analytics of Graph Algorithms using Intel Optane DC Persistent Memory\n");
	printf("by Evan Unmann\n");

	printf("PMEM:\n");
	printf("\tVersion: %d.%d\n", PMEM_MAJOR_VERSION, PMEM_MINOR_VERSION);

	/* Test the allocator to see if PMEM is accessible or not */
	PMEM::allocator<float> allocator;
	const size_t N = 1e6;
	auto p = allocator.allocate(N);
	printf("\tPMEM Accessible: %s\n", allocator.is_pmem() ? "True" : "False");

	for (size_t i = 0; i < N; i++) {
		p[i] = 1.0f;
	}

	printf("VALUE: %f\n", p[0]);
	allocator.deallocate(p, N);

	printf("OpenMP:\n");
	printf("\tNumber of Processors: %d\n", omp_get_num_procs());
	printf("\tMaximum Threads: %d\n", omp_get_max_threads());
}

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");

	try {

		print_info();

		return 0;

		Benchmark::Parameters tp = Benchmark::get_parameters();

		std::vector<std::pair<std::string, std::string>> graph_paths;
		graph_paths.push_back(std::make_pair("./graph_examples/facebook_combined.txt", "Facebook"));
		graph_paths.push_back(std::make_pair("./graph_examples/soc-Epinions1.txt", "Epinions"));
		graph_paths.push_back(std::make_pair("./graph_examples/soc-pokec-relationships.txt", "Pokec"));
		graph_paths.push_back(std::make_pair("./graph_examples/sx-stackoverflow.txt", "Stack Overflow"));
		graph_paths.push_back(std::make_pair("./graph_examples/soc-LiveJournal1.txt", "Live Journal"));
		graph_paths.push_back(std::make_pair("./graph_examples/com-orkut.ungraph.txt", "Orkut"));
		//graph_paths.push_back(std::make_pair("./graph_examples/com-friendster.ungraph.txt", "Friendster"));

		for (const auto& graph_path : graph_paths) {
			tp.graph_path = graph_path.first;
			tp.graph_name = graph_path.second;

			GraphCRS<std::allocator> graph_dram;
			GraphCRS<PMEM::allocator> graph_pmem;

			printf("Loading graph from %s\n", tp.graph_path.c_str());
			{
				BlockTimer timer("Graph DRAM Load");
				graph_dram = GraphUtils::load<std::allocator>(tp.graph_path);
			}
			{
				BlockTimer timer("Graph PMEM Load");
				graph_pmem = GraphUtils::load<PMEM::allocator>(tp.graph_path);
			}

			Benchmark::benchmark_page_rank(tp, graph_dram, graph_pmem);
			Benchmark::benchmark_page_rank_sizes(tp, graph_dram, graph_pmem);
			Benchmark::benchmark_breadth_first_traversal(tp, graph_dram, graph_pmem);

			graph_dram.free();
			graph_pmem.free();
		}

		//Benchmark::benchmark_memory(tp);
		//Benchmark::benchmark_STREAM(tp);
	}
	catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}

	return 0;
}