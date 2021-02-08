#include "BenchmarkUtils.hpp"

#include <functional>
#include <random>

namespace BenchmarkUtils {

	std::string get_graph_name(const std::string& graph_path) {
		size_t start_index = graph_path.find_last_of("/") + 1;
		size_t end_index = graph_path.find_last_of(".") - start_index;
		return graph_path.substr(start_index, end_index);
	}

	void set_random_values(char* arr, size_t size) {
#pragma omp parallel
		{
			std::default_random_engine generator;
			std::uniform_int_distribution<char> distribution(0, std::numeric_limits<char>::max());
			auto value_gen = std::bind(distribution, generator);
#pragma omp for
			for (size_t i = 0; i < size; i++) {
				arr[i] = value_gen();
			}
		}
	}
}