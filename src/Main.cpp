#include <stdio.h>
#include "Benchmarks.hpp"
#include "BlockTimer.hpp"
#include <string>
#include <omp.h>
#include <exception>
#include <libpmem.h>
#include "GraphUtils.hpp"

void print_info() {

	printf("PMEM:\n");
	printf("\tVersion: %d.%d\n", PMEM_MAJOR_VERSION, PMEM_MINOR_VERSION);

	printf("OpenMP:\n");
	printf("\tNumber of Processors: %d\n", omp_get_num_procs());
	printf("\tMaximum Threads: %d\n", omp_get_max_threads());
}

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");
	printf("Persistent Memory Benchmark\n");
	printf("by Evan Unmann\n");

	print_info();

	try {

		Benchmark::Parameters tp = Benchmark::get_parameters();

		std::vector<std::string> graph_paths;

		graph_paths.push_back("./graph_examples/facebook_combined.txt");
		graph_paths.push_back("./graph_examples/soc-Epinions1.txt");
		graph_paths.push_back("./graph_examples/soc-pokec-relationships.txt");
		graph_paths.push_back("./graph_examples/com-orkut.ungraph.txt");
		graph_paths.push_back("./graph_examples/soc-LiveJournal1.txt");
		graph_paths.push_back("./graph_examples/sx-stackoverflow.txt");

		for (const auto& graph_path : graph_paths) {
			tp.graph_path = graph_path;
			Benchmark::benchmark_page_rank(tp);
			Benchmark::benchmark_breadth_first_traversal(tp);
		}

		Benchmark::benchmark_memory(tp);
	}
	catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}

	return 0;
}