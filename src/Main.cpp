#include <stdio.h>
#include "Tests.hpp"
#include "BlockTimer.hpp"
#include <string>
#include <omp.h>
#include <exception>
#include <cstdlib>
#include "FormatUtils.hpp"
#include "GNUPlot/Plot.hpp"
#include <libpmem.h>
#include "GraphUtils.hpp"

void print_info() {

	printf("PMEM:\n");
	printf("\tVersion: %d.%d\n", PMEM_MAJOR_VERSION, PMEM_MINOR_VERSION);

	printf("OpenMP:\n");
	printf("\tNumber of Processors: %d\n", omp_get_num_procs());
	printf("\tMaximum Threads: %d\n", omp_get_max_threads());
}

void test_graph_load() {

	std::string path = "./graph_examples/soc-Epinions1.txt";
	GraphCRS g1 = GraphUtils::load(path);
	g1.page_rank(100, 0.8f);

	printf("DRAM\n");
	printf("Vertices: %u\n", g1.num_vertices());
	printf("Edges:    %u\n", g1.num_edges());

	PMEM::GraphCRS g2 = GraphUtils::load_as_pmem(path, "./tmp/");
	g2.page_rank(100, 0.8f);

	printf("PMEM\n");
	printf("Vertices: %u\n", g2.num_vertices());
	printf("Edges:    %u\n", g2.num_edges());
}

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");
	printf("Persistent Memory Benchmark\n");
	printf("by Evan Unmann\n");

	print_info();

	try {

		Tests::TestParameters tp = Tests::get_test_parameters();

		std::vector<std::string> graph_paths = {
			"./graph_examples/Slashdot0902.txt",
			"./graph_examples/soc-Epinions1.txt",
			"./graph_examples/soc-LiveJournal1.txt",
			"./graph_examples/soc-pokec-relationships.txt"
		};

		for (auto& path : graph_paths) {
			tp.graph_path = path;
			Tests::graph_test_page_rank(tp);
			Tests::graph_test_breadth_first_traversal(tp);
		}

		Tests::pmem_vs_dram_benchmark(tp);
	}
	catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}

	return 0;
}