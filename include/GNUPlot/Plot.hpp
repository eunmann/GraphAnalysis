#pragma once

#include <string>
#include <vector>

namespace GNUPlot {

	void save_csv(const std::string& path, const std::vector<std::string>& keys, const std::vector<std::vector<double>>& data);

	void save_plot_command(const std::string& image_path, const std::string& title, size_t width, size_t height, const std::vector<std::string>& keys, const std::vector<std::string>& axies_labels, const std::vector<std::vector<double>>& data);

	void plot_save_png(const std::string& path, const std::string& title, size_t width, size_t height, const std::vector<std::string>& keys, const std::vector<std::string>& axies_labels, const std::vector<std::vector<double>>& data);

	std::string plot_command(const std::string& image_path, const std::string& title, size_t width, size_t height, double min, double max, const std::string& data_path, size_t num_cols, const std::vector<std::string>& axies_labels);
}