#include "GraphGenerator.hpp"
#include <random>
#include <algorithm>

GraphCRS GraphGenerator::create_graph_crs(uint32_t num_vertices, uint32_t max_degree, const float min_value, const float max_value) {

	/* Create a function to generate random degrees for each vertex */
	std::default_random_engine gen_degree;
	std::uniform_int_distribution<uint32_t> dis_degree(1, max_degree);
	auto random_degree = std::bind(dis_degree, gen_degree);

	/* Create a function to generate random values for edges */
	std::default_random_engine gen_val;
	std::uniform_real_distribution<float> dis_val(min_value, max_value);
	auto random_val = std::bind(dis_val, gen_val);

	/* Create a function to generate random indexes for vertices */
	std::default_random_engine gen_col_index;
	std::uniform_int_distribution<uint32_t> dis_col_index(0, num_vertices - 1);
	auto random_index = std::bind(dis_col_index, gen_col_index);

	std::vector<float> val;
	std::vector<uint32_t> col_ind;
	std::vector<uint32_t> row_ind;

	uint32_t rpi = 0;
	for (uint32_t r = 0; r < num_vertices; r++) {

		/* Create random edges */
		uint32_t degree = random_degree();
		std::vector<uint32_t> rand_col_index;
		for (uint32_t d = 0; d < degree; d++) {
			rand_col_index.push_back(random_index());
		}

		/* Sort the indexes and remove duplicates */
		std::sort(rand_col_index.begin(), rand_col_index.end());
		rand_col_index.erase(std::unique(rand_col_index.begin(), rand_col_index.end()), rand_col_index.end());

		/* Store the column indexes and generate random values for the edges */
		for (auto i : rand_col_index) {
			col_ind.push_back(i);
			val.push_back(random_val());
		}

		/* Set the row pointer index */
		row_ind.push_back(rpi);
		rpi += rand_col_index.size();
	}

	return GraphCRS(val, col_ind, row_ind);
}

PMEM::GraphCRS GraphGenerator::create_graph_crs_pmem(uint32_t num_vertices, uint32_t max_degree, const float min_value, const float max_value) {

	/* Create a function to generate random degrees for each vertex */
	std::default_random_engine gen_degree;
	std::uniform_int_distribution<uint32_t> dis_degree(1, max_degree);
	auto random_degree = std::bind(dis_degree, gen_degree);

	/* Create a function to generate random values for edges */
	std::default_random_engine gen_val;
	std::uniform_real_distribution<float> dis_val(min_value, max_value);
	auto random_val = std::bind(dis_val, gen_val);

	/* Create a function to generate random indexes for vertices */
	std::default_random_engine gen_col_index;
	std::uniform_int_distribution<uint32_t> dis_col_index(0, num_vertices - 1);
	auto random_index = std::bind(dis_col_index, gen_col_index);

	std::string path = "/tmp/";
	PMEM::vector<float> val(path, num_vertices);
	PMEM::vector<uint32_t> col_ind(path, num_vertices);
	PMEM::vector<uint32_t> row_ind(path, num_vertices);

	uint32_t rpi = 0;
	for (uint32_t r = 0; r < num_vertices; r++) {

		/* Create random edges */
		uint32_t degree = random_degree();
		std::vector<uint32_t> rand_col_index;
		for (uint32_t d = 0; d < degree; d++) {
			rand_col_index.push_back(random_index());
		}

		/* Sort the indexes and remove duplicates */
		std::sort(rand_col_index.begin(), rand_col_index.end());
		rand_col_index.erase(std::unique(rand_col_index.begin(), rand_col_index.end()), rand_col_index.end());

		/* Store the column indexes and generate random values for the edges */
		for (auto i : rand_col_index) {
			col_ind.push_back(i);
			val.push_back(random_val());
		}

		/* Set the row pointer index */
		row_ind.push_back(rpi);
		rpi += rand_col_index.size();
	}

	return PMEM::GraphCRS(val, col_ind, row_ind);
}