#include <stdio.h>
#include "Tests.hpp"
#include "BlockTimer.hpp"
#include <string>
#include <omp.h>
#include <exception>
#include <cstdlib>
#include "FormatUtils.hpp"
#include "GNUPlot/Plot.hpp"

void print_info() {
	printf("OpenMP:\n");
	printf("\tNumber of Processors: %d\n", omp_get_num_procs());
	printf("\tMaximum Threads: %d\n", omp_get_max_threads());
}

void test_plot() {
	std::vector<std::vector<double>> data;
	data.push_back(std::vector<double>());
	data.push_back(std::vector<double>());
	std::vector<double>& x_values = data[0];
	std::vector<double>& y_values = data[1];

	for (int i = 0; i < 10; i++) {
		x_values.push_back(i);
		y_values.push_back(i + 5);
	}

	printf("X[0]=%f Data[0]=%f\n", x_values[0], data[0][0]);
	x_values[0] = 20;
	printf("X[0]=%f Data[0]=%f\n", x_values[0], data[0][0]);

	GNUPlot::save_plot_command("./gnuplot_test.png", "Test Plot", 800, 600, { "X Values", "Y Values" }, { "Iteration", "Time" }, data);
}

int main(int argc, char** argv) {
	BlockTimer timer("Time Elapsed");
	printf("Graph Algorithm Performance Analysis on Persistent Memory Machines\n");
	printf("by Evan Unmann\n");

	print_info();

	size_t alloc_size = 1;
	uint32_t num_vertices = 1;

	if (std::getenv("alloc_size") != nullptr) {
		alloc_size = std::stol(std::getenv("alloc_size"));
	}

	if (std::getenv("num_vertices") != nullptr) {
		num_vertices = std::stol(std::getenv("num_vertices"));
	}

	printf("Memory Test:\n");
	printf("\talloc_size: %sB\n", FormatUtils::format_number(alloc_size).c_str());

	printf("Graph Test:\n");
	printf("\tnum_vertices: %s\n", FormatUtils::format_number(num_vertices).c_str());

	try {
		//Tests::graph_test_page_rank(num_vertices);
		Tests::graph_test_breadth_first_traversal(num_vertices);
		//Tests::pmem_vs_dram_benchmark(alloc_size);
	}
	catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}

	return 0;
}