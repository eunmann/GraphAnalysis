#include "PMEM/GraphCRS.hpp"

#include <iostream>
#include <fstream>
#include <queue>

namespace PMEM {

	GraphCRS::GraphCRS(std::string directory) : GraphCRS::GraphCRS(PMEM::vector<float>(directory, 256),
		PMEM::vector<uint32_t>(directory, 256),
		PMEM::vector<uint32_t>(directory, 256)) {};

	GraphCRS::GraphCRS(PMEM::vector<float> val,
		PMEM::vector<uint32_t> col_ind,
		PMEM::vector<uint32_t> row_ind) : val(val),
		col_ind(col_ind),
		row_ind(row_ind) {
	}

	const float GraphCRS::weight(const uint32_t i, const uint32_t j) const {
		return this->val[this->index(i, j)];
	}

	float& GraphCRS::operator()(const uint32_t i, const uint32_t j) {
		return this->val[this->index(i, j)];
	}

	void GraphCRS::set(const float weight, const uint32_t i, const uint32_t j) {
		this->val[this->index(i, j)] = weight;
	}

	const uint32_t GraphCRS::index(const uint32_t i, const uint32_t j) const {
		for (uint32_t rs = this->row_ind[i]; rs < this->col_ind.size(); rs++) {
			if (col_ind[rs] == j) {
				return rs;
			}
		}

		return -1;
	}

	void GraphCRS::save(const std::string& path) const {

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


	std::string GraphCRS::to_string() const {
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

	void GraphCRS::print() const {
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

	std::vector<float> GraphCRS::page_rank(size_t iterations, float dampening_factor) const {

		/* Note this vector is in DRAM and not PMEM */
		float init_prob = 1.0f / this->row_ind.size();
		float init_dampening_prob = (1.0f - dampening_factor) / this->row_ind.size();

		/* Use two vectors since the next iteration relies on the current iteration */
		std::vector<float> page_rank_vec_1(this->row_ind.size(), init_prob);
		std::vector<float> page_rank_vec_2(this->row_ind.size(), init_prob);

		for (size_t i = 0; i < iterations; i++) {

			std::vector<float>& page_rank_read_vec = i % 2 == 0 ? page_rank_vec_1 : page_rank_vec_2;
			std::vector<float>& page_rank_write_vec = i % 2 == 1 ? page_rank_vec_1 : page_rank_vec_2;

#pragma omp parallel for
			for (size_t vertex = 0; vertex < this->row_ind.size(); vertex++) {
				uint32_t row_index = this->row_ind[vertex];
				uint32_t row_index_end = vertex + 1 == this->row_ind.size() ? this->col_ind.size() : this->row_ind[vertex + 1];

				float page_rank_sum = 0;

				/* For each neighbor */
				for (; row_index < row_index_end; row_index++) {
					uint32_t neighbor = this->col_ind[row_index];
					float dist = this->val[row_index];

					page_rank_sum += page_rank_read_vec[neighbor] / dist;
				}

				page_rank_sum = page_rank_sum * dampening_factor + init_dampening_prob;
				page_rank_write_vec[vertex] = page_rank_sum;
			}
		}

		return iterations % 2 == 0 ? page_rank_vec_1 : page_rank_vec_2;
	}

	void GraphCRS::breadth_first_traversal(uint32_t vertex) const {

		std::queue<uint32_t> frontier;
		frontier.push(vertex);

		std::vector<uint32_t> visited(this->num_vertices(), 0);

		while (!frontier.empty()) {

#pragma omp parallel for
			for (size_t i = 0; i < frontier.size(); i++) {

				uint32_t vertex;
#pragma omp critical
				{
					vertex = frontier.front();
					frontier.pop();
				}

				uint32_t row_index_end = (vertex + 1 == this->row_ind.size()) ? this->col_ind.size() : this->row_ind[vertex + 1];

				/* For each neighbor */
				for (uint32_t row_index = this->row_ind[vertex]; row_index < row_index_end; row_index++) {
					uint32_t neighbor = this->col_ind[row_index];
					if (visited[neighbor] == 0) {
						visited[neighbor] = 1;
#pragma omp critical
						{
							frontier.push(neighbor);
						}
					}
				}
			}
		}
	}

	uint32_t GraphCRS::num_edges() const {
		return this->val.size();
	}

	uint32_t GraphCRS::num_vertices() const {
		return this->row_ind.size();
	}

	size_t GraphCRS::byte_size() const {
		return sizeof(float) * (this->col_ind.size() + this->row_ind.size() + this->val.size());
	}

	bool GraphCRS::is_pmem() const {
		return this->col_ind.is_pmem() && this->row_ind.is_pmem() && this->val.is_pmem();
	}
}
