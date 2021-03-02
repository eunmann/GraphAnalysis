#include "BenchmarkUtils.hpp"

#include <functional>
#include <random>
#include <fstream>

namespace BenchmarkUtils {

	void set_zeros(char* arr, size_t size) {
#pragma omp parallel for schedule(STATIC)
		for (size_t i = 0; i < size; i++) {
			arr[i] = 0;
		}
	}

	void create_csv(const std::string& path, const std::vector<std::string>& headers) {
		std::ofstream ofs(path);
		if (ofs.is_open()) {
			for (auto& header : headers) {
				ofs << header;
				ofs << ',';
			}
			ofs.seekp(-1, std::ios::cur);
			ofs << std::endl;
		}
		ofs.close();
	}

	void save_graph_metrics_csv(const std::string& path, const std::string& graph_name, const std::vector<double>& dram_metrics, const std::vector<double>& pmem_metrics) {

		double dram_min;
		double dram_max;
		double dram_avg;
		double dram_std;

		BenchmarkUtils::metrics(dram_metrics, dram_min, dram_max, dram_avg, dram_std);

		double pmem_min;
		double pmem_max;
		double pmem_avg;
		double pmem_std;

		BenchmarkUtils::metrics(pmem_metrics, pmem_min, pmem_max, pmem_avg, pmem_std);

		std::ofstream ofs(path, std::ios::app);
		if (ofs.is_open()) {
			ofs << graph_name;
			ofs << ',';

			ofs << FormatUtils::format_number(dram_avg);
			ofs << ',';

			ofs << FormatUtils::format_number(pmem_avg);
			ofs << std::endl;
		}
		ofs.close();
	}

	void save_mem_metrics_csv(const std::string& path, const std::string& benchmark_name, const std::string& unit, const std::vector<double>& dram_metrics, const std::vector<double>& pmem_metrics) {
		double dram_min;
		double dram_max;
		double dram_avg;
		double dram_std;

		BenchmarkUtils::metrics(dram_metrics, dram_min, dram_max, dram_avg, dram_std);

		double pmem_min;
		double pmem_max;
		double pmem_avg;
		double pmem_std;

		BenchmarkUtils::metrics(pmem_metrics, pmem_min, pmem_max, pmem_avg, pmem_std);

		std::ofstream ofs(path, std::ios::app);
		if (ofs.is_open()) {
			ofs << benchmark_name;
			ofs << ',';

			ofs << FormatUtils::format_number(dram_avg);
			ofs << unit;
			ofs << ',';

			ofs << FormatUtils::format_number(pmem_avg);
			ofs << unit;
			ofs << std::endl;
		}
		ofs.close();
	}

	void save_mem_metrics_csv(const std::string& path, const std::string& benchmark_name, const std::string& unit, double dram_metric, double pmem_metric) {
		std::ofstream ofs(path, std::ios::app);
		if (ofs.is_open()) {
			ofs << benchmark_name;
			ofs << ',';

			ofs << FormatUtils::format_number(dram_metric);
			ofs << unit;
			ofs << ',';

			ofs << FormatUtils::format_number(pmem_metric);
			ofs << unit;
			ofs << std::endl;
		}
		ofs.close();
	}
}