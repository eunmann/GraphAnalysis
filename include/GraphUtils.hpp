#pragma once

#include "GraphCRS.hpp"
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>

namespace GraphUtils {

	template<template<class> class T>
	GraphCRS<T> create_graph_crs(uint32_t num_vertices, uint32_t min_degree, uint32_t max_degree, float min_value, float max_value) {
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

		GraphCRS<T> graph(row_ind.size(), val.size());

		for (size_t i = 0; i < graph.num_edges(); i++) {
			graph.val[i] = val[i];
			graph.col_ind[i] = col_ind[i];
		}

		for (size_t i = 0; i < graph.num_vertices(); i++) {
			graph.row_ind[i] = row_ind[i];
		}

		return graph;
	}

	template<template<class> class T>
	GraphCRS<T> load(const std::string& path) {
		std::ifstream file(path, std::ios::binary);
		if (!file) {
			printf("Could not open file at %s\n", path.c_str());
			return GraphCRS<T>(0, 0);
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

			GraphCRS<T> graph(row_ind.size(), val.size());

			for (size_t i = 0; i < graph.num_edges(); i++) {
				graph.val[i] = val[i];
				graph.col_ind[i] = col_ind[i];
			}

			for (size_t i = 0; i < graph.num_vertices(); i++) {
				graph.row_ind[i] = row_ind[i];
			}

			return graph;
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

			file.close();

			GraphCRS<T> graph(row_ind.size(), val.size());

			for (size_t i = 0; i < graph.num_edges(); i++) {
				graph.val[i] = val[i];
				graph.col_ind[i] = col_ind[i];
			}

			for (size_t i = 0; i < graph.num_vertices(); i++) {
				graph.row_ind[i] = row_ind[i];
			}

			return graph;
		}
		else if (std::equal(path.end() - 4, path.end(), ".txt")) {
			/*
				This file format is typically a tab seperated value format, where the first
				column is the source node, and the second column is the destination node.
				This is a link to get graphs of this format from real world sources:
				https://snap.stanford.edu/data/index.html
			*/

			std::string line;
			std::getline(file, line);

			/* The first line should describe whether the graph is directed or not */
			bool is_directed = line.rfind("# Directed", 0) == 0;

			/* Lines with comments are marks with a '#', so skip these */
			while (line[0] == '#') {
				std::getline(file, line);
			}

			std::vector<std::vector<uint32_t>> node_map;
			size_t num_edges = 0;

			/* Read each pair */
			do {
				std::istringstream iss(line);
				uint32_t source, destination;
				if (!(iss >> source >> destination)) {
					break;
				}

				uint32_t max_node_id = source > destination ? source : destination;

				/* Make sure there is a vector for this node */
				if (node_map.size() < max_node_id + 1) {
					node_map.resize(max_node_id + 1);
				}

				node_map[source].push_back(destination);

				if (!is_directed) {
					node_map[destination].push_back(source);
					num_edges += 2;
				}
				else {
					num_edges++;
				}
			} while (std::getline(file, line));

			file.close();

			/* There's no values for edge weights, so just default to 1 */
			std::vector<float> val(num_edges, 1);
			std::vector<uint32_t> col_ind;
			std::vector<uint32_t> row_ind;

			/* Preallocate memory */
			col_ind.reserve(num_edges);
			row_ind.reserve(node_map.size());

			for (auto& node_v : node_map) {

				/* To create the CSR graph, sort the vectors */
				std::sort(node_v.begin(), node_v.end());

				/* Set the index for the current row */
				row_ind.push_back(col_ind.size());

				/* Append the vector to col_ind */
				col_ind.insert(col_ind.end(), node_v.begin(), node_v.end());
			}

			GraphCRS<T> graph(row_ind.size(), val.size());

			for (size_t i = 0; i < graph.num_edges(); i++) {
				graph.val[i] = val[i];
				graph.col_ind[i] = col_ind[i];
			}

			for (size_t i = 0; i < graph.num_vertices(); i++) {
				graph.row_ind[i] = row_ind[i];
			}

			return graph;
		}
		else {
			printf("File type not supported: %s\n", path.substr(path.length() - 4).c_str());
			return GraphCRS<T>(0, 0);
		}
	}

	template<template<class> class T>
	std::string to_string(const GraphCRS<T>& graph) {
		std::string output = "";

		/* First row, val */
		uint32_t i = 0;
		for (uint32_t e = graph.num_edges() - 1; i < e; i++) {
			output += std::to_string(graph.val[i]);
			output += ',';
		}
		output += std::to_string(graph.val[i]);
		output += '\n';

		/* Second row, col_ind */
		i = 0;
		for (uint32_t e = graph.num_edges() - 1; i < e; i++) {
			output += std::to_string(graph.col_ind[i]);
			output += ',';
		}
		output += std::to_string(graph.col_ind[i]);
		output += '\n';

		/* Third row, row_ind */
		i = 0;
		for (uint32_t e = graph.num_vertices() - 1; i < e; i++) {
			output += std::to_string(graph.row_ind[i]);
			output += ',';
		}
		output += std::to_string(graph.row_ind[i]);
		output += '\n';

		return output;
	}

	template<template<class> class T>
	void save(const GraphCRS<T>& graph, const std::string& path) {

		std::ofstream file(path, std::ios::binary);

		if (file) {
			if (std::equal(path.end() - 4, path.end(), ".csv")) {
				std::string output = GraphUtils::to_string(graph);
				file.write(output.c_str(), output.size());
				file.close();
			}
			else {
				/* Write the vector size, then the vector data */
				size_t size = graph.num_edges();
				file.write(reinterpret_cast<const char*>(&size), sizeof(size));
				file.write(reinterpret_cast<const char*>(&graph.val[0]), sizeof(float) * size);

				file.write(reinterpret_cast<const char*>(&size), sizeof(size));
				file.write(reinterpret_cast<const char*>(&graph.col_ind[0]), sizeof(uint32_t) * size);

				size = graph.num_vertices();
				file.write(reinterpret_cast<const char*>(&size), sizeof(size));
				file.write(reinterpret_cast<const char*>(&graph.row_ind[0]), sizeof(uint32_t) * size);
			}

			file.close();
		}
	}
};