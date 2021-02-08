#pragma once

#include <vector>
#include <cmath>
#include "FormatUtils.hpp"

namespace BenchmarkUtils {

	template<class T>
	void metrics(const std::vector<T>& vec, double& min, double& max) {
		min = vec[0];
		max = vec[0];

		for (auto& v : vec) {

			if (v < min) {
				min = v;
			}
			else if (v > max) {
				max = v;
			}
		}
	}

	template<class T>
	void metrics(const std::vector<T>& vec, double& min, double& max, double& avg, double& std_dev) {

		min = vec[0];
		max = vec[0];
		avg = 0;

		for (auto& v : vec) {

			if (v < min) {
				min = v;
			}
			else if (v > max) {
				max = v;
			}

			avg += v;
		}

		avg /= vec.size();

		std_dev = 0;

		for (auto& v : vec) {
			double diff = v - avg;
			std_dev += diff * diff;
		}
		std_dev /= vec.size();
		std_dev = std::sqrt(std_dev);
	}

	template<class T>
	void print_metrics(const std::string& title, const std::vector<T>& vec) {
		double min;
		double max;
		double avg;
		double std_dev;

		BenchmarkUtils::metrics(vec, min, max, avg, std_dev);
		printf("%s:\n", title.c_str());
		printf("\tMinimum: %s\n", FormatUtils::format_number(min).c_str());
		printf("\tMaximum: %s\n", FormatUtils::format_number(max).c_str());
		printf("\tAverage: %s\n", FormatUtils::format_number(avg).c_str());
		printf("\tSTD_DEV: %s\n", FormatUtils::format_number(std_dev).c_str());
	}

	template<class T>
	void compare_metrics(const std::vector<T>& vec_a, const std::vector<T>& vec_b) {

		double min_a;
		double max_a;
		double avg_a;
		double std_a;

		BenchmarkUtils::metrics(vec_a, min_a, max_a, avg_a, std_a);

		double min_b;
		double max_b;
		double avg_b;
		double std_b;

		BenchmarkUtils::metrics(vec_b, min_b, max_b, avg_b, std_b);
		printf("Minimum: %s, %s, %s, %.2f%%\n", FormatUtils::format_number(min_a).c_str(), FormatUtils::format_number(min_b).c_str(), FormatUtils::format_number(min_b - min_a).c_str(), ((min_b / min_a) - 1.0) * 100.0);
		printf("Maximum: %s, %s, %s, %.2f%%\n", FormatUtils::format_number(max_a).c_str(), FormatUtils::format_number(max_b).c_str(), FormatUtils::format_number(max_b - max_a).c_str(), ((max_b / max_a) - 1.0) * 100.0);
		printf("Average: %s, %s, %s, %.2f%%\n", FormatUtils::format_number(avg_a).c_str(), FormatUtils::format_number(avg_b).c_str(), FormatUtils::format_number(avg_b - avg_a).c_str(), ((avg_b / avg_a) - 1.0) * 100.0);
		printf("STD_DEV: %s, %s, %s, %.2f%%\n", FormatUtils::format_number(std_a).c_str(), FormatUtils::format_number(std_b).c_str(), FormatUtils::format_number(std_b - std_a).c_str(), ((std_b / std_a) - 1.0) * 100.0);
	}

	std::string get_graph_name(const std::string& graph_path);
	void set_random_values(char* arr, size_t size);
}