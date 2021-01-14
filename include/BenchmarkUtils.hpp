#pragma once

#include <vector>
#include <cmath>

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
		printf("\tMinimum: %.3f\n", min);
		printf("\tMaximum: %.3f\n", max);
		printf("\tAverage: %.3f\n", avg);
		printf("\tSTD_DEV: %.3f\n", std_dev);
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
		printf("Minimum: %.3f, %.3f, %.3f, %.3f\n", min_a, min_b, min_b - min_a, (min_b / min_a));
		printf("Maximum: %.3f, %.3f, %.3f, %.3f\n", max_a, max_b, max_b - max_a, (max_b / max_a));
		printf("Average: %.3f, %.3f, %.3f, %.3f\n", avg_a, avg_b, avg_b - avg_a, (avg_b / avg_a));
		printf("STD_DEV: %.3f, %.3f, %.3f, %.3f\n", std_a, std_b, std_b - std_a, (std_b / std_a));
	}
}