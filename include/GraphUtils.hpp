#pragma once

#include "GraphCRS.hpp"
#include "PMEM/GraphCRS.hpp"

namespace GraphUtils {
	GraphCRS create_graph_crs(uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, float min_value, float max_value);
	PMEM::GraphCRS create_graph_crs_pmem(const std::string& directory, uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, float min_value, float max_value);
	GraphCRS load(const std::string& path);
	PMEM::GraphCRS load_as_pmem(const std::string& path, const std::string& directory);
};