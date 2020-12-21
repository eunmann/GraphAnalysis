#pragma once

#include <string>
#include <vector>

namespace GNUPlot {

	void plot_save_png(std::string path, std::string title, size_t width, size_t height, std::vector<std::string>& headers, std::vector<std::vector<double>>& data);

	std::string plot_command(std::string image_path, std::string title, size_t width, size_t height, double min, double max, std::string data_path);
}