#include "GraphCRS.hpp"


class GraphGenerator {

public:
	static GraphCRS create_graph_crs(uint32_t num_vertices, uint32_t max_degree, const float min_value, const float max_value);
};