#pragma once

#include "GraphCRS.hpp"
#include "PMEM/GraphCRS.hpp"

namespace GraphUtils {
	GraphCRS create_graph_crs(uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, const float min_value, const float max_value);
	PMEM::GraphCRS create_graph_crs_pmem(std::string directory, uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, const float min_value, const float max_value);
	GraphCRS load(std::string path);
	PMEM::GraphCRS load_as_pmem(std::string path, std::string directory);
};