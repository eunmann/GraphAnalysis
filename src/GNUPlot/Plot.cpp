#include "GNUPlot/Plot.hpp"
#include "BenchmarkUtils.hpp"
#include <chrono>
#include <fstream>
#include <cstdio>

namespace GNUPlot {

	void plot_save_png(const std::string& path, const std::string& title, size_t width, size_t height, const std::vector<std::string>& headers, const std::vector<std::vector<double>>& data) {

		/* Find the minimum and maximum values from the vectors */
		double min = data[0][0];
		double max = data[0][0];

		for (const std::vector<double>& vec : data) {
			double min_v;
			double max_v;

			BenchmarkUtils::metrics(vec, min_v, max_v);

			if (min_v < min) {
				min = min_v;
			}

			if (max_v > max) {
				max = max_v;
			}
		}

		/* Create and write the data to a temporary file */
		std::string data_path = std::string("./tmp/data_file_") + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
		std::ofstream data_file(data_path, std::ofstream::binary);

		for (const std::string& header : headers) {
			data_file.write(header.c_str(), header.size());
			data_file.write(",", 1);
		}

		data_file.seekp(1, std::ios_base::beg);
		data_file.write("\n", 1);

		const size_t num_data = data[0].size();
		for (size_t i = 0; i < num_data; i++) {

			for (auto& vec : data) {
				std::string s = std::to_string(vec[i]);
				data_file.write(s.c_str(), s.size());
				data_file.write(",", 1);
			}

			data_file.seekp(1, std::ios_base::beg);
			data_file.write("\n", 1);
		}

		data_file.close();

		std::string command = GNUPlot::plot_command(path, title, width, height, min, max, data_path, data.size());
		int rc = system(command.c_str());

		if (rc == 1) {
			printf("GNUPlot Failed: rc=%d\n", rc);
		}

		std::remove(data_path.c_str());
	}

	std::string plot_command(const std::string& image_path, const std::string& title, size_t width, size_t height, double min, double max, const std::string& data_path, size_t num_cols) {
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

		/* Set the plot */
		command += "plot for [col=1:";
		command += std::to_string(num_cols);
		command += "] [";
		command += std::to_string(min);
		command += ":";
		command += std::to_string(max);
		command += "] \'";
		command += data_path;
		command += "\' using 0:col with lines";

		/* End of GNUPlot command quote */
		command += "\"";

		return command;
	}
}