#include "PMEM/GraphCRS.hpp"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace PMEM {

	GraphCRS::GraphCRS(PMEM::vector<float> val,
		PMEM::vector<uint32_t> col_ind,
		PMEM::vector<uint32_t> row_ind) : val(val),
		col_ind(col_ind),
		row_ind(row_ind) {
	}

	const float GraphCRS::weight(const uint32_t i, const uint32_t j) {
		return this->val[this->index(i, j)];
	}

	float& GraphCRS::operator()(const uint32_t i, const uint32_t j) {
		return this->val[this->index(i, j)];
	}

	void GraphCRS::set(const float weight, const uint32_t i, const uint32_t j) {
		this->val[this->index(i, j)] = weight;
	}

	void GraphCRS::for_each(std::function<void(float& v, const uint32_t i, const uint32_t j)> func) {

		for (uint32_t i = 0; i < this->row_ind.size(); i++) {
			uint32_t re = i == this->row_ind.size() - 1 ? this->col_ind.size() : this->row_ind[i + 1];
			for (uint32_t j = this->row_ind[i];j < re; j++) {
				func((*this)(i, j), i, j);
			}
		}
	}
	const uint32_t GraphCRS::index(const uint32_t i, const uint32_t j) {
		for (uint32_t rs = this->row_ind[i]; rs < this->col_ind.size(); rs++) {
			if (col_ind[rs] == j) {
				return rs;
			}
		}

		return -1;
	}

	void GraphCRS::save(std::string path) {

		std::ofstream file(path, std::ios::binary);

		if (file) {
			if (std::equal(path.end() - 4, path.end(), ".csv")) {
				std::string output = this->to_string();
				file.write(output.c_str(), output.size());
				file.close();
			}
			else {
				/* Write the vector size, then the vector data */
				uint32_t size = this->val.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(reinterpret_cast<char*>(&this->val[0]), sizeof(float) * this->val.size());

				size = this->col_ind.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(reinterpret_cast<char*>(&this->col_ind[0]), sizeof(uint32_t) * this->col_ind.size());

				size = this->row_ind.size();
				file.write(reinterpret_cast<char*>(&size), sizeof(size));
				file.write(reinterpret_cast<char*>(&this->row_ind[0]), sizeof(uint32_t) * this->row_ind.size());
			}

			file.close();
		}
	}


	std::string GraphCRS::to_string() {
		std::string output = "";

		/* First row, val */
		uint32_t i = 0;
		for (uint32_t e = this->val.size() - 1; i < e; i++) {
			output += std::to_string(this->val[i]);
			output += ',';
		}
		output += std::to_string(this->val[i]);
		output += '\n';

		/* Second row, col_ind */
		i = 0;
		for (uint32_t e = this->col_ind.size() - 1; i < e; i++) {
			output += std::to_string(this->col_ind[i]);
			output += ',';
		}
		output += std::to_string(this->col_ind[i]);
		output += '\n';

		/* Third row, row_ind */
		i = 0;
		for (uint32_t e = this->row_ind.size() - 1; i < e; i++) {
			output += std::to_string(this->row_ind[i]);
			output += ',';
		}
		output += std::to_string(this->row_ind[i]);
		output += '\n';

		return output;
	}

	void GraphCRS::print() {
		std::string output = "";
		std::string zero_str = std::to_string(0);

		for (uint32_t r = 0, r_e = this->row_ind.size() - 1; r < r_e; r++) {
			uint32_t r_i = this->row_ind[r];
			uint32_t r_i_e = this->row_ind[r + 1];
			for (uint32_t c = 0; c < this->row_ind.size(); c++) {
				if (r_i < r_i_e && this->col_ind[r_i] == c) {
					output += std::to_string(this->val[r_i]);
					r_i++;
				}
				else {
					output += zero_str;
				}
				output += ',';
			}
			output += '\n';
		}

		uint32_t r_i = this->row_ind[this->row_ind.size() - 1];
		for (uint32_t c = 0; c < this->row_ind.size(); c++) {
			if (this->col_ind[r_i] == c) {
				output += std::to_string(this->val[r_i]);
				r_i++;
			}
			else {
				output += zero_str;
			}
			output += ',';
		}
		output += '\n';

		printf("%s", output.c_str());
	}

	void GraphCRS::free() {
		this->val.free();
		this->col_ind.free();
		this->row_ind.free();
	}

	std::vector<uint32_t> GraphCRS::shortest_path(uint32_t source, uint32_t destination) {

		/* Distance Map */
		std::unordered_map<uint32_t, float> dist_map;
		/* Traversal Map*/
		std::unordered_map<uint32_t, uint32_t> trav_map;
		/* Visited Set */
		std::unordered_set<uint32_t> visited_set;
		/* Priority Queue ordered on smallest to large distance */
		auto dist_cmp = [&dist_map](const uint32_t left, const uint32_t right) { return dist_map[left] < dist_map[right];};
		std::vector<uint32_t> search_queue;

		/* Initialize the source node*/
		dist_map[source] = 0;
		search_queue.push_back(source);

		/* Go through all of the nodes in the queue */
		while (!search_queue.empty()) {

			/* Get the node with the curently shortest path */
			uint32_t node = search_queue.front();
			search_queue.erase(search_queue.begin());

			/* The destination was the current shortest path */
			if (node == destination) {
				break;
			}

			float dist = dist_map[node];
			uint32_t row_index = this->row_ind[node];
			uint32_t row_index_end = node == this->row_ind.size() ? this->col_ind.size() : this->row_ind[node + 1];

			/* For each neighbor */
			for (; row_index < row_index_end; row_index++) {
				uint32_t neighbor = this->col_ind[row_index];
				float combined_dist = dist + this->val[row_index];
				if (dist_map.find(neighbor) == dist_map.end() || combined_dist < dist_map[neighbor]) {
					dist_map[neighbor] = combined_dist;
					trav_map[neighbor] = node;
				}

				if (visited_set.find(neighbor) == visited_set.end()) {

					/* Find and remove the neighbor if it was already queued */
					auto pos = std::find(search_queue.begin(), search_queue.end(), neighbor);
					if (pos != search_queue.end()) {
						search_queue.erase(pos);
					}
					/* Add the neighbor into the queue */
					pos = std::upper_bound(search_queue.begin(), search_queue.end(), neighbor, dist_cmp);
					search_queue.insert(pos, neighbor);
				}
			}

			visited_set.insert(node);
		}

		/* Use the traversal map to build the path from destination to source */
		std::vector<uint32_t> path;
		uint32_t target = destination;
		if (trav_map.find(target) != trav_map.end() || target == source) {
			while (target != source) {
				path.push_back(target);
				target = trav_map[target];
			}
			path.push_back(source);
		}

		std::reverse(path.begin(), path.end());

		return path;
	}
}
