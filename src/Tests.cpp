#include "PMEM/Tests.hpp"
#include <stdio.h>
#include "GraphUtils.hpp"
#include <random>
#include "Timer.hpp"
#include "BlockTimer.hpp"
#include "FormatUtils.hpp"
#include "BenchmarkUtils.hpp"
#include "GNUPlot/Plot.hpp"
#include "Tests.hpp"
#include <immintrin.h>

#define GNUPLOT_WIDTH 800
#define GNUPLOT_HEIGHT 600

namespace Tests {

	Tests::TestParameters get_test_parameters() {
		TestParameters tp;

		if (std::getenv("alloc_size") != nullptr) {
			tp.alloc_size = std::stol(std::getenv("alloc_size"));
		}

		if (std::getenv("num_vertices") != nullptr) {
			tp.num_vertices = std::stol(std::getenv("num_vertices"));
		}

		if (std::getenv("min_degree") != nullptr) {
			tp.min_degree = std::stol(std::getenv("min_degree"));
		}

		if (std::getenv("max_degree") != nullptr) {
			tp.max_degree = std::stol(std::getenv("max_degree"));
		}

		if (std::getenv("min_value") != nullptr) {
			tp.min_value = std::stol(std::getenv("min_value"));
		}

		if (std::getenv("max_value") != nullptr) {
			tp.max_value = std::stol(std::getenv("max_value"));
		}

		if (std::getenv("page_rank_iterations") != nullptr) {
			tp.page_rank_iterations = std::stol(std::getenv("page_rank_iterations"));
		}

		if (std::getenv("page_rank_dampening_factor") != nullptr) {
			tp.page_rank_dampening_factor = std::stod(std::getenv("page_rank_dampening_factor"));
		}

		if (std::getenv("test_iterations") != nullptr) {
			tp.test_iterations = std::stol(std::getenv("test_iterations"));
		}

		if (std::getenv("pmem_directory") != nullptr) {
			tp.pmem_directory = std::string(std::getenv("pmem_directory"));
		}

		printf("Test Parameters:\n");
		printf("\talloc_size: %sB\n", FormatUtils::format_number(tp.alloc_size).c_str());
		printf("\tnum_vertices: %s\n", FormatUtils::format_number(tp.num_vertices).c_str());
		printf("\tmin_degree: %s\n", FormatUtils::format_number(tp.min_degree).c_str());
		printf("\tmax_degree: %s\n", FormatUtils::format_number(tp.max_degree).c_str());
		printf("\tmin_value: %s\n", FormatUtils::format_number(tp.min_value).c_str());
		printf("\tmax_value: %s\n", FormatUtils::format_number(tp.max_value).c_str());
		printf("\tpage_rank_iterations: %s\n", FormatUtils::format_number(tp.page_rank_iterations).c_str());
		printf("\tpage_rank_dampening_factor: %s\n", FormatUtils::format_number(tp.page_rank_dampening_factor).c_str());
		printf("\ttest_iterations: %s\n", FormatUtils::format_number(tp.test_iterations).c_str());
		printf("\tGraph Path: %s\n", tp.graph_path.c_str());
		printf("\tPMEM Directory: %s\n", tp.pmem_directory.c_str());

		return tp;
	}

	void graph_test(const Tests::TestParameters& tp) {

		std::string graph_file_path = "./graph_examples/test_format.csv";
		std::string simple_graph_path = "./graph_examples/simple_graph.csv";

		printf("First Graph\n");
		PMEM::GraphCRS g1 = GraphUtils::create_graph_crs_pmem("/tmp/", tp.num_vertices, tp.min_degree, tp.max_degree, tp.min_value, tp.max_value);
		g1.print();
		g1.save(graph_file_path);
		g1.free();

		printf("Second Graph\n");
		PMEM::GraphCRS g2 = GraphUtils::load_as_pmem(simple_graph_path, "/tmp/");
		g2.print();
		std::vector<uint32_t> shortest_path = g2.shortest_path(2, 4);

		if (shortest_path.size() > 0 && shortest_path.size() <= 5) {

			printf("[");
			for (size_t i = 0; i < shortest_path.size() - 1; i++) {
				printf(" %u ->", shortest_path[i]);
			}
			printf(" %u ]\n", shortest_path.back());
		}
		else {
			printf("Shortest Path has wrong size [%lu]\n", shortest_path.size());
		}
		g2.free();
	}

	void graph_test_page_rank(const Tests::TestParameters& tp) {

		printf("Page Rank\n");
		printf("\tNumber of Vertices: %u\n", tp.num_vertices);
		printf("\tMinimum Degree: %u\n", tp.min_degree);
		printf("\tMaximum Degree: %u\n", tp.max_degree);
		printf("\tPage Rank Iterations: %u\n", tp.page_rank_iterations);
		printf("\tDampening Factor: %f\n", tp.page_rank_dampening_factor);
		printf("\tTest Iterations: %u\n", tp.test_iterations);

		std::string temp_graph_path = "./tmp/page_rank_graph.csv";

		std::vector<std::vector<double>> time_elapsed(2);
		std::vector<std::vector<double>> edges_per_second(2);

		{
			printf("DRAM\n");
			GraphCRS graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs(tp.num_vertices, tp.min_degree, tp.max_degree, tp.min_value, tp.max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				graph.save(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load(tp.graph_path);
			}

			printf("Number of Vertices: %u\n", graph.num_vertices());
			printf("Number of Edges: %u\n", graph.num_edges());
			printf("Memory Size: %lu B\n", graph.byte_size());

			std::vector<double>& time_elapsed_v = time_elapsed[0];
			printf("Iteration, Time Elapsed (s), Edges per Second\n");
			for (uint32_t i = 1; i <= tp.test_iterations; i++) {
				Timer timer;
				graph.page_rank(tp.page_rank_iterations, tp.page_rank_dampening_factor);
				timer.end();

				double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
				time_elapsed_v.push_back(time_elapsed_seconds);
				printf("%d,%.3f,%.3f\n", i, time_elapsed_seconds, graph.num_edges() / time_elapsed_seconds);
			}

			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double>& edges_per_second_v = edges_per_second[0];
			for (auto& t : time_elapsed_v) {
				edges_per_second_v.push_back(graph.num_edges() / t);
			}
			BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);
		}

		{
			printf("PMEM\n");
			PMEM::GraphCRS graph_pmem(tp.pmem_directory);

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(temp_graph_path, tp.pmem_directory);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(tp.graph_path, tp.pmem_directory);
			}

			printf("Number of Vertices: %u\n", graph_pmem.num_vertices());
			printf("Number of Edges: %u\n", graph_pmem.num_edges());
			printf("Memory Size: %lu B\n", graph_pmem.byte_size());
			printf("Is pmem: %s\n", graph_pmem.is_pmem() ? "True" : "False");
			std::vector<double>& time_elapsed_v = time_elapsed[1];
			printf("Iteration, Time Elapsed (s), Edges per Second\n");
			for (uint32_t i = 1; i <= tp.test_iterations; i++)
			{
				Timer timer;
				graph_pmem.page_rank(tp.page_rank_iterations, tp.page_rank_dampening_factor);
				timer.end();

				double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
				time_elapsed_v.push_back(time_elapsed_seconds);
				printf("%d,%.3f,%.3f\n", i, time_elapsed_seconds, graph_pmem.num_edges() / time_elapsed_seconds);
			}
			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double>& edges_per_second_v = edges_per_second[1];
			for (auto& t : time_elapsed_v) {
				edges_per_second_v.push_back(graph_pmem.num_edges() / t);
			}
			BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);
			graph_pmem.free();
		}

		std::string graph_name = Tests::get_graph_name(tp.graph_path);
		GNUPlot::save_plot_command(std::string(std::getenv("out_dir")) + "page_rank_time." + graph_name + ".png", "Page Rank", GNUPLOT_WIDTH, GNUPLOT_HEIGHT, { "DRAM", "PMEM" }, { "Iteration", "Time (s)" }, time_elapsed);
		GNUPlot::save_plot_command(std::string(std::getenv("out_dir")) + "page_rank_edges." + graph_name + ".png", "Page Rank", GNUPLOT_WIDTH, GNUPLOT_HEIGHT, { "DRAM", "PMEM" }, { "Iteration", "Edges per Second" }, edges_per_second);
	}

	void graph_test_breadth_first_traversal(const Tests::TestParameters& tp) {

		printf("Breadth First Traversal\n");
		printf("\tNumber of Vertices: %u\n", tp.num_vertices);
		printf("\tMinimum Degree: %u\n", tp.min_degree);
		printf("\tMaximum Degree: %u\n", tp.max_degree);
		printf("\tTest Iterations: %u\n", tp.test_iterations);

		std::string temp_graph_path = "./tmp/bfs_graph.csv";

		std::vector<std::vector<double>> time_elapsed(2);
		std::vector<std::vector<double>> edges_per_second(2);

		/* Create a list of vertices so the tests perform the same traversals */
		std::vector<uint32_t> start_vertices;

		{
			printf("DRAM\n");
			GraphCRS graph;

			if (tp.graph_path.empty()) {
				printf("Generating graph\n");
				graph = GraphUtils::create_graph_crs(tp.num_vertices, tp.min_degree, tp.max_degree, tp.min_value, tp.max_value);
				printf("Saving graph to %s\n", temp_graph_path.c_str());
				graph.save(temp_graph_path);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph = GraphUtils::load(tp.graph_path);
			}

			for (uint32_t i = 0; i < tp.test_iterations; i++) {
				start_vertices.push_back(graph.num_vertices() * double(i) / double(tp.test_iterations));
			}

			printf("Number of Vertices: %u\n", graph.num_vertices());
			printf("Number of Edges: %u\n", graph.num_edges());
			printf("Memory Size: %lu B\n", graph.byte_size());

			std::vector<double>& time_elapsed_v = time_elapsed[0];
			printf("Iteration, Time Elapsed (s), Edges per Second\n");
			for (uint32_t i = 1; i <= tp.test_iterations; i++) {
				Timer timer;
				graph.breadth_first_traversal(start_vertices[i - 1]);
				timer.end();

				double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
				time_elapsed_v.push_back(time_elapsed_seconds);
				printf("%d,%.3f,%.3f\n", i, time_elapsed_seconds, graph.num_edges() / time_elapsed_seconds);
			}

			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double>& edges_per_second_v = edges_per_second[0];
			for (auto& t : time_elapsed_v) {
				edges_per_second_v.push_back(graph.num_edges() / t);
			}
			BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);
		}

		{
			printf("PMEM\n");
			PMEM::GraphCRS graph_pmem(tp.pmem_directory);

			if (tp.graph_path.empty()) {
				printf("Loading graph from %s\n", temp_graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(temp_graph_path, tp.pmem_directory);
			}
			else {
				printf("Loading graph from %s\n", tp.graph_path.c_str());
				graph_pmem = GraphUtils::load_as_pmem(tp.graph_path, tp.pmem_directory);
			}

			printf("Number of Vertices: %u\n", graph_pmem.num_vertices());
			printf("Number of Edges: %u\n", graph_pmem.num_edges());
			printf("Memory Size: %lu B\n", graph_pmem.byte_size());
			printf("Is pmem: %s\n", graph_pmem.is_pmem() ? "True" : "False");
			std::vector<double>& time_elapsed_v = time_elapsed[1];
			printf("Iteration, Time Elapsed (s), Edges per Second\n");
			for (uint32_t i = 1; i <= tp.test_iterations; i++)
			{
				Timer timer;
				graph_pmem.breadth_first_traversal(start_vertices[i - 1]);
				timer.end();

				double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
				time_elapsed_v.push_back(time_elapsed_seconds);
				printf("%d,%.3f,%.3f\n", i, time_elapsed_seconds, graph_pmem.num_edges() / time_elapsed_seconds);
			}
			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double>& edges_per_second_v = edges_per_second[1];
			for (auto& t : time_elapsed_v) {
				edges_per_second_v.push_back(graph_pmem.num_edges() / t);
			}
			BenchmarkUtils::print_metrics("Edges per Second", edges_per_second_v);
			graph_pmem.free();
		}

		std::string graph_name = Tests::get_graph_name(tp.graph_path);
		GNUPlot::save_plot_command(std::string(std::getenv("out_dir")) + "bfs_time." + graph_name + ".png", "BFS", GNUPLOT_WIDTH, GNUPLOT_HEIGHT, { "DRAM", "PMEM" }, { "Iteration", "Time (s)" }, time_elapsed);
		GNUPlot::save_plot_command(std::string(std::getenv("out_dir")) + "bfs_edges." + graph_name + ".png", "BFS", GNUPLOT_WIDTH, GNUPLOT_HEIGHT, { "DRAM", "PMEM" }, { "Iteration", "Edges per Second" }, edges_per_second);
	}

	std::vector<std::vector<double>> memory_benchmark(char* arr, const size_t size) {

		int iter_per_test = 10;
		const size_t latency_loads = size / 20;
		printf("Memory Benchmark\n");
		printf("Memory Size: %sB\n", FormatUtils::format_number(size).c_str());
		printf("Latency Loads: %sB\n", FormatUtils::format_number(latency_loads).c_str());
		printf("Iterations per Test: %d\n", iter_per_test);

		double size_gigabytes = size / 1e9;
		auto print_bandwidth = [&size_gigabytes](const int iteration, const Timer& timer) {
			double time_elapsed_seconds = timer.get_time_elapsed() / 1e9;
			printf("%d,%.3f,%.3f\n", iteration, time_elapsed_seconds, size_gigabytes / time_elapsed_seconds);
		};

		auto print_latency = [&latency_loads](const int iteration, const Timer& timer) {
			printf("%d,%.3f,%.3f\n", iteration, timer.get_time_elapsed() / 1e9, 1.0 * timer.get_time_elapsed() / latency_loads);
		};

		auto print_vec_bandwith = [&size_gigabytes](const std::vector<double> time_elapsed_v) {
			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double> bandwidth_v;
			for (auto& t : time_elapsed_v) {
				bandwidth_v.push_back(size_gigabytes / t);
			}
			BenchmarkUtils::print_metrics("Bandwidth", bandwidth_v);
		};

		auto print_vec_latency = [&latency_loads](const std::vector<double> time_elapsed_v) {
			BenchmarkUtils::print_metrics("Time Elapsed", time_elapsed_v);

			std::vector<double> latency_v;
			for (auto& t : time_elapsed_v) {
				latency_v.push_back(1e9 * t / latency_loads);
			}
			BenchmarkUtils::print_metrics("Latency", latency_v);
		};

		std::vector<std::vector<double>> metric_v(4);
		BlockTimer timer("Memory Test");

		/* Read linear */
		{
			printf("Read Linear\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (GB/s)\n");
			std::vector<double> time_elapsed_v;

			int64_t sum = 0;
			const int64_t* test_mem = (int64_t*)arr;
			const size_t test_mem_size = size / sizeof(int64_t);

			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;
#pragma omp parallel for
				for (size_t i = 0; i < test_mem_size; i++) {
					sum += test_mem[i];
					sum *= sum;
				}
				timer.end();
				print_bandwidth(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}
			print_vec_bandwith(time_elapsed_v);
			printf("IGNORE(%ld)\n", sum);

			/* Bandwidth */
			for (auto& time_elapsed : time_elapsed_v) {
				metric_v[0].push_back((double)size / time_elapsed);
			}
		}

		/* Read Random */
		{
			printf("Read Random\n");
			printf("Iteration, Time Elapsed (s), Latency (ns)\n");
			std::vector<double> time_elapsed_v;
			std::default_random_engine generator;
			std::uniform_int_distribution<uint64_t> distribution(0, size);
			auto indexGen = std::bind(distribution, generator);

			char sum = 0;
			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;

				for (size_t i = 0; i < latency_loads; i++) {
					sum += arr[indexGen()];
				}

				timer.end();
				print_latency(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}
			print_vec_latency(time_elapsed_v);
			printf("IGNORE(%c)\n", sum);

			/* Average Latency */
			for (auto& time_elapsed : time_elapsed_v) {
				metric_v[1].push_back(1e9 * time_elapsed / (double)latency_loads);
			}
		}

		/* Write linear */
		{
			printf("Write Linear\n");
			printf("Iteration, Time Elapsed (s), Bandwidth (GB/s)\n");
			std::vector<double> time_elapsed_v;
			uint64_t* test_mem = (uint64_t*)arr;
			const size_t test_mem_size = size / sizeof(uint64_t);

			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;
#pragma omp parallel for
				for (size_t i = 0; i < test_mem_size; i++) {
					test_mem[i] = 0;
				}
				timer.end();
				print_bandwidth(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}
			print_vec_bandwith(time_elapsed_v);
			/* Bandwidth */
			for (auto& time_elapsed : time_elapsed_v) {
				metric_v[2].push_back((double)size / time_elapsed);
			}
		}

		/* Write Random */
		{
			printf("Write Random\n");
			printf("Iteration, Time Elapsed (s), Latency (ns)\n");

			std::default_random_engine generator;
			std::uniform_int_distribution<uint64_t> distribution(0, size);
			auto indexGen = std::bind(distribution, generator);

			std::vector<double> time_elapsed_v;
			for (int iter = 1; iter <= iter_per_test; iter++) {
				Timer timer;

				for (size_t i = 0; i < latency_loads; i++) {
					arr[indexGen()] = 0;
				}

				timer.end();
				print_latency(iter, timer);
				time_elapsed_v.push_back(timer.get_time_elapsed() / 1e9);
			}

			print_vec_latency(time_elapsed_v);

			/* Average Latency */
			for (auto& time_elapsed : time_elapsed_v) {
				metric_v[3].push_back(1e9 * time_elapsed / (double)latency_loads);
			}
		}

		return metric_v;
	}

	void pmem_vs_dram_benchmark(const Tests::TestParameters& tp) {
		printf("DRAM\n");
		char* dram_array = new char[tp.alloc_size];
		/* Touch the first and last element to make sure the OS allocated the memory */
		printf("IGNORE(%c)\n", dram_array[0] + dram_array[tp.alloc_size - 1]);
		std::vector<std::vector<double>> dram_time_elapsed = memory_benchmark(dram_array, tp.alloc_size);
		delete dram_array;

		printf("Persistent Memory\n");
		PMEM::ptr pmem = PMEM::ptr(tp.pmem_directory, PMEM::FILE::TEMP, tp.alloc_size);
		char* pmem_array = pmem.as<char*>();

		printf("Is pmem: %s\n", pmem.is_pmem() ? "True" : "False");
		printf("Mapped length: %sB\n", FormatUtils::format_number(pmem.mapped_len()).c_str());

		if (pmem_array == nullptr) {
			printf("Trouble allocating persistent memory\n");
			return;
		}
		/* Touch the first and last element to make sure the OS allocated the memory */
		printf("IGNORE(%c)\n", pmem_array[0] + pmem_array[tp.alloc_size - 1]);
		std::vector<std::vector<double>> pmem_time_elapsed = memory_benchmark(pmem_array, tp.alloc_size);
		pmem.free();

		const std::vector<std::string> paths({ "read_linear.png", "read_random.png", "write_linear.png", "write_random.png" });
		const std::vector<std::string> titles({ "Read Linear", "Read Random", "Write Linear", "Write Random" });
		const std::vector<std::string> labels({ "Bandwidth (B/s)", "Latency (ns)", "Bandwidth (B/s)", "Latency (ns)" });
		for (size_t i = 0; i < dram_time_elapsed.size(); i++) {
			std::vector<std::vector<double>> data;
			data.push_back(dram_time_elapsed[i]);
			data.push_back(pmem_time_elapsed[i]);
			GNUPlot::save_plot_command(std::string(std::getenv("out_dir")) + paths[i], titles[i], GNUPLOT_WIDTH, GNUPLOT_HEIGHT, { "DRAM", "PMEM" }, { "Iteration", labels[i] }, data);
		}
	}

	std::string get_graph_name(const std::string& graph_path) {
		size_t start_index = graph_path.find_last_of("/") + 1;
		size_t end_index = graph_path.find_last_of(".") - start_index;
		return graph_path.substr(start_index, end_index);
	}
}