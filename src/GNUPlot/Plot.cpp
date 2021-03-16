#include "GNUPlot/Plot.hpp"
#include "BenchmarkUtils.hpp"
#include <chrono>
#include <fstream>
#include <cstdio>

namespace GNUPlot {

	void save_csv(const std::string& path, const std::vector<std::string>& keys, const std::vector<std::vector<double>>& data) {

		std::ofstream file(path, std::ofstream::binary);

		for (const std::string& header : keys) {
			file.write(header.c_str(), header.size());
			file.write(",", 1);
		}

		file.seekp(-1, std::ios_base::cur);
		file.write("\n", 1);

		const size_t num_data = data[0].size();
		for (size_t i = 0; i < num_data; i++) {

			for (auto& vec : data) {
				std::string s = std::to_string(vec[i]);
				file.write(s.c_str(), s.size());
				file.write(",", 1);
			}

			file.seekp(-1, std::ios_base::cur);
			file.write("\n", 1);
		}

		file.close();
	}

	void save_plot_command(const std::string& image_path, const std::string& title, size_t width, size_t height, const std::vector<std::string>& keys, const std::vector<std::string>& axies_labels, const std::vector<std::vector<double>>& data) {
		/* Find the minimum and maximum values from the vectors */
		double min = data[0][0];
		double max = data[0][0];

		for (auto& vec : data) {
			double min_v;
			double max_v;
			double avg;

			BenchmarkUtils::metrics(vec, min_v, max_v, avg);

			if (min_v < min) {
				min = min_v;
			}

			if (max_v > max) {
				max = max_v;
			}
		}

		std::string data_path = std::string(std::getenv("out_dir")) + "data_file_0.csv";

		/*
		int i = 0;
		while (std::filesystem::exists(data_path)) {
			i++;
			data_path = std::string(std::getenv("out_dir")) + "data_file_" + std::to_string(i) + ".csv";
		}
		*/

		GNUPlot::save_csv(data_path, keys, data);
		std::string command = GNUPlot::plot_command(image_path, title, width, height, min, max, data_path, data.size(), axies_labels);

		std::string command_file_path = std::string(std::getenv("out_dir")) + "gnuplot_command.sh";
		std::ofstream file(command_file_path, std::ofstream::binary | std::ofstream::app);
		file << command << std::endl;
		file.close();

		// std::filesystem::permissions(command_file_path, std::filesystem::perms::all);
	}


	void plot_save_png(const std::string& path, const std::string& title, size_t width, size_t height, const std::vector<std::string>& keys, const std::vector<std::string>& axies_labels, const std::vector<std::vector<double>>& data) {

		/* Find the minimum and maximum values from the vectors */
		double min = data[0][0];
		double max = data[0][0];

		for (auto& vec : data) {
			double min_v;
			double max_v;
			double avg;

			BenchmarkUtils::metrics(vec, min_v, max_v, avg);

			if (min_v < min) {
				min = min_v;
			}

			if (max_v > max) {
				max = max_v;
			}
		}

		std::string data_path = std::string(std::getenv("out_dir")) + "data_file_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + ".csv";
		GNUPlot::save_csv(data_path, keys, data);

		std::string command = GNUPlot::plot_command(path, title, width, height, min, max, data_path, data.size(), axies_labels);
		int rc = system(command.c_str());

		if (rc == 1) {
			printf("GNUPlot Failed: rc=%d\n", rc);
		}

		std::remove(data_path.c_str());
	}

	std::string plot_command(const std::string& image_path, const std::string& title, size_t width, size_t height, double min, double max, const std::string& data_path, size_t num_cols, const std::vector<std::string>& axies_labels) {
		std::string command = "gnuplot -e ";

		/* Wrap the GNUPlot commands in quotes */
		command += "\"";

		/* Set the terminal size */
		command += "set terminal png size ";
		command += std::to_string(width);
		command += ",";
		command += std::to_string(height);
		command += "; ";

		/* Set the output path */
		command += "set output \'";
		command += image_path;
		command += "\'; ";

		/* Set the title */
		command += "set title \'";
		command += title;
		command += "\'; ";

		/* Set the column headers */
		command += "set key autotitle columnheader; ";

		/* Set the separator */
		command += "set datafile separator \',\'; ";

		/* Set key outside */
		command += "set key outside; ";

		/* Set the axis labels */
		command += "set xlabel \'";
		command += axies_labels[0];
		command += "\'; ";
		command += "set ylabel \'";
		command += axies_labels[1];
		command += "\'; ";

		/* Set the format for y values */
		command += "set format y \'%.2s%c\'; ";

		/* Set the plot */
		command += "plot for [col=1:";
		command += std::to_string(num_cols);
		command += "] [";
		command += std::to_string(min);
		command += ":";
		command += std::to_string(max);
		command += "] \'";
		command += data_path;
		command += "\' using 0:col with linespoints pt 20 ps 2";

		/* End of GNUPlot command quote */
		command += "\"";

		return command;
	}
}