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

void print_info() {

	printf("PMEM:\n");
	printf("\tVersion: %d.%d\n", PMEM_MAJOR_VERSION, PMEM_MINOR_VERSION);

	/* Test the allocator to see if PMEM is accessible or not */
	PMEM::allocator<float> allocator;
	const size_t N = 10;
	auto p = allocator.allocate(N);
	printf("\tPMEM Accessible: %s\n", allocator.is_pmem() ? "True" : "False");
	allocator.deallocate(p, N);

	printf("OpenMP:\n");
	printf("\tNumber of Processors: %d\n", omp_get_num_procs());
	printf("\tMaximum Threads: %d\n", omp_get_max_threads());
}

void init_csv_files(const Benchmark::Parameters& tp) {
	BenchmarkUtils::create_csv(tp.pr_csv_path, { "graph", "DD", "DP", "PD", "PP" });
	BenchmarkUtils::create_csv(tp.bfs_csv_path, { "graph", "DD", "DP", "PD", "PP" });
	BenchmarkUtils::create_csv(tp.mem_csv_path, { "benchmark", "dram", "pmem" });
}

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");
	printf("Performance Analytics of Graph Algorithms using Intel Optane DC Persistent Memory\n");
	printf("by Evan Unmann\n");

	print_info();

	try {

		Benchmark::Parameters tp = Benchmark::get_parameters();
		init_csv_files(tp);

		std::vector<std::pair<std::string, std::string>> graph_paths;

		graph_paths.push_back(std::make_pair("./graph_examples/facebook_combined.txt", "Facebook"));
		graph_paths.push_back(std::make_pair("./graph_examples/soc-Epinions1.txt", "Epinions"));
		graph_paths.push_back(std::make_pair("./graph_examples/soc-pokec-relationships.txt", "Pokec"));
		graph_paths.push_back(std::make_pair("./graph_examples/soc-LiveJournal1.txt", "Live Journal"));
		graph_paths.push_back(std::make_pair("./graph_examples/sx-stackoverflow.txt", "Stack Overflow"));
		//graph_paths.push_back(std::make_pair("./graph_examples/com-orkut.ungraph.txt", "Orkut"));
		//graph_paths.push_back(std::make_pair("./graph_examples/com-friendster.ungraph.txt", "Friendster"));
		//graph_paths.clear();


		for (const auto& graph_path : graph_paths) {
			tp.graph_path = graph_path.first;
			tp.graph_name = graph_path.second;

			//Benchmark::benchmark_page_rank(tp);
			Benchmark::benchmark_breadth_first_traversal(tp);
		}

		//Benchmark::benchmark_memory(tp);
		//Benchmark::benchmark_STREAM(tp);
	}
	catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}

	return 0;
}