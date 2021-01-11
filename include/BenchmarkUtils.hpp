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
}