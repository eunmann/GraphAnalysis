#pragma once

#include <string>
#include <vector>

namespace GNUPlot {

	void plot_save_png(const std::string& path, const std::string& title, size_t width, size_t height, const std::vector<std::string>& headers, const std::vector<std::vector<double>>& data);

	std::string plot_command(const std::string& image_path, const std::string& title, size_t width, size_t height, double min, double max, const std::string& data_path, size_t num_cols);
}