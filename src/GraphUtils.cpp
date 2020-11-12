#include "GraphUtils.hpp"
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>

namespace GraphUtils {

	GraphCRS create_graph_crs(uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, const float min_value, const float max_value) {

		/* Create a function to generate random degrees for each vertex */
		std::default_random_engine gen_degree;
		std::uniform_int_distribution<uint32_t> dis_degree(min_degree, max_degree);
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
			uint32_t ri;
			for (uint32_t d = 0; d < degree; d++) {
				/* Avoid creating edges from a node to itself */
				while ((ri = random_index()) == r);
				rand_col_index.push_back(ri);
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

	PMEM::GraphCRS create_graph_crs_pmem(std::string directory, uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, const float min_value, const float max_value) {

		/* Create a function to generate random degrees for each vertex */
		std::default_random_engine gen_degree;
		std::uniform_int_distribution<uint32_t> dis_degree(min_degree, max_degree);
		auto random_degree = std::bind(dis_degree, gen_degree);

		/* Create a function to generate random values for edges */
		std::default_random_engine gen_val;
		std::uniform_real_distribution<float> dis_val(min_value, max_value);
		auto random_val = std::bind(dis_val, gen_val);

		/* Create a function to generate random indexes for vertices */
		std::default_random_engine gen_col_index;
		std::uniform_int_distribution<uint32_t> dis_col_index(0, num_vertices - 1);
		auto random_index = std::bind(dis_col_index, gen_col_index);

		PMEM::vector<float> val(directory, num_vertices);
		PMEM::vector<uint32_t> col_ind(directory, num_vertices);
		PMEM::vector<uint32_t> row_ind(directory, num_vertices);

		uint32_t rpi = 0;
		for (uint32_t r = 0; r < num_vertices; r++) {

			/* Create random edges */
			uint32_t degree = random_degree();
			std::vector<uint32_t> rand_col_index;
			uint32_t ri;
			for (uint32_t d = 0; d < degree; d++) {
				/* Avoid creating edges from a node to itself */
				while ((ri = random_index()) == r);
				rand_col_index.push_back(ri);
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

	GraphCRS load(std::string path) {
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			printf("Could not open file at %s\n", path.c_str());
			return GraphCRS(std::vector<float>(), std::vector<uint32_t>(), std::vector<uint32_t>());
		}

		if (std::equal(path.end() - 4, path.end(), ".csv")) {
			/*
				TODO(EMU): For larger graphs, this needs to be implemented differently.
				Loading the entire file into memory and then converting it might take up
				too much memory.
			*/
			file.seekg(0, std::ios::end);
			std::string input;
			input.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(&input[0], input.size());
			file.close();

			std::vector<float> val;
			size_t i = 0;
			size_t end = input.find('\n');
			while (i < end) {
				size_t sep_index = input.find(',', i);
				sep_index = sep_index > end ? end : sep_index;
				size_t char_len = sep_index - i;
				val.push_back(std::stof(input.substr(i, char_len)));
				i = sep_index + 1;
			}

			std::vector<uint32_t> col_ind;
			col_ind.reserve(val.size());
			end = input.find('\n', i);
			while (i < end) {
				size_t sep_index = input.find(',', i);
				sep_index = sep_index > end ? end : sep_index;
				size_t char_len = sep_index - i;
				col_ind.push_back(std::stoi(input.substr(i, char_len)));
				i = sep_index + 1;
			}

			std::vector<uint32_t> row_ind;
			end = input.size();
			while (i < end) {
				size_t sep_index = input.find(',', i);
				sep_index = sep_index > end ? end : sep_index;
				size_t char_len = sep_index - i;
				row_ind.push_back(std::stoi(input.substr(i, char_len)));
				i = sep_index + 1;
			}

			return GraphCRS(val, col_ind, row_ind);
		}
		else if (std::equal(path.end() - 4, path.end(), ".crs")) {
			uint32_t size;
			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			std::vector<float> val;
			val.reserve(size);
			file.read(reinterpret_cast<char*>(&val[0]), sizeof(float) * size);

			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			std::vector<uint32_t> col_ind;
			col_ind.reserve(size);
			file.read(reinterpret_cast<char*>(&col_ind[0]), sizeof(uint32_t) * size);

			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			std::vector<uint32_t> row_ind;
			row_ind.reserve(size);
			file.read(reinterpret_cast<char*>(&row_ind[0]), sizeof(uint32_t) * size);

			return GraphCRS(val, col_ind, row_ind);
		}
		else {
			printf("File type not supported: %s\n", path.substr(path.length() - 4).c_str());
			return GraphCRS(std::vector<float>(), std::vector<uint32_t>(), std::vector<uint32_t>());
		}
	}

	PMEM::GraphCRS load_as_pmem(std::string path, std::string directory) {
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			printf("Could not open file at %s\n", path.c_str());
			return PMEM::GraphCRS(PMEM::vector<float>(directory, 0), PMEM::vector<uint32_t>(directory, 0), PMEM::vector<uint32_t>(directory, 0));
		}

		if (std::equal(path.end() - 4, path.end(), ".csv")) {
			/*
				TODO(EMU): For larger graphs, this needs to be implemented differently.
				Loading the entire file into memory and then converting it might take up
				too much memory.
			*/
			file.seekg(0, std::ios::end);
			std::string input;
			input.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(&input[0], input.size());
			file.close();

			PMEM::vector<float> val(directory, 64);
			size_t i = 0;
			size_t end = input.find('\n');
			while (i < end) {
				size_t sep_index = input.find(',', i);
				sep_index = sep_index > end ? end : sep_index;
				size_t char_len = sep_index - i;
				val.push_back(std::stof(input.substr(i, char_len)));
				i = sep_index + 1;
			}

			PMEM::vector<uint32_t> col_ind(directory, val.size());
			end = input.find('\n', i);
			while (i < end) {
				size_t sep_index = input.find(',', i);
				sep_index = sep_index > end ? end : sep_index;
				size_t char_len = sep_index - i;
				col_ind.push_back(std::stoi(input.substr(i, char_len)));
				i = sep_index + 1;
			}

			PMEM::vector<uint32_t> row_ind(directory, 64);
			end = input.size();
			while (i < end) {
				size_t sep_index = input.find(',', i);
				sep_index = sep_index > end ? end : sep_index;
				size_t char_len = sep_index - i;
				row_ind.push_back(std::stoi(input.substr(i, char_len)));
				i = sep_index + 1;
			}

			return PMEM::GraphCRS(val, col_ind, row_ind);
		}
		else if (std::equal(path.end() - 4, path.end(), ".crs")) {
			uint32_t size;
			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			PMEM::vector<float> val(directory, size);
			file.read(reinterpret_cast<char*>(&val[0]), sizeof(float) * size);

			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			PMEM::vector<uint32_t> col_ind(directory, size);
			file.read(reinterpret_cast<char*>(&col_ind[0]), sizeof(uint32_t) * size);

			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			PMEM::vector<uint32_t> row_ind(directory, size);
			file.read(reinterpret_cast<char*>(&row_ind[0]), sizeof(uint32_t) * size);

			return PMEM::GraphCRS(val, col_ind, row_ind);
		}
		else {
			printf("File type not supported: %s\n", path.substr(path.length() - 4).c_str());
			return PMEM::GraphCRS(PMEM::vector<float>(directory, 0), PMEM::vector<uint32_t>(directory, 0), PMEM::vector<uint32_t>(directory, 0));
		}
	}
}