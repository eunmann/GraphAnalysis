#include "PMEM/GraphCRS.hpp"

#include <iostream>
#include <fstream>
#include <queue>
#include "InstructionUtils.hpp"

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

	float GraphCRS::weight(const uint32_t i, const uint32_t j) const {
		return this->val[this->index(i, j)];
	}

	float& GraphCRS::operator()(const uint32_t i, const uint32_t j) {
		return this->val[this->index(i, j)];
	}

	void GraphCRS::set(const float weight, const uint32_t i, const uint32_t j) {
		this->val[this->index(i, j)] = weight;
	}

	uint32_t GraphCRS::index(const uint32_t i, const uint32_t j) const {
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

	std::vector<std::vector<float>> GraphCRS::page_rank(size_t iterations, std::vector<float> dampening_factors) const {

		const size_t num_dampening_factors = dampening_factors.size();
		const float init_prob = 1.0f / this->row_ind.size();
		std::vector<float> dampening_probs;
		for (auto& dampening_factor : dampening_factors) {
			dampening_probs.push_back((1.0f - dampening_factor) / this->row_ind.size());
		}

		/* Use two vectors since the next iteration relies on the current iteration */
		std::vector<std::vector<float>> prv_1;
		std::vector<std::vector<float>> prv_2;
		for (size_t i = 0; i < num_dampening_factors; i++) {
			prv_1.push_back(std::vector<float>(this->row_ind.size(), init_prob));
			prv_2.push_back(std::vector<float>(this->row_ind.size(), init_prob));
		}

		/* Compute the number of adjacent vertices inverse for each vertex */
		std::vector<float> adjacent_vertices_inv(this->row_ind.size());
		const size_t e = adjacent_vertices_inv.size();
#pragma omp parallel for
		for (size_t i = 0; i < e; i++) {
			uint32_t row_index = this->row_ind[i];
			uint32_t row_index_end = i + 1 == this->row_ind.size() ? this->col_ind.size() : this->row_ind[i + 1];
			adjacent_vertices_inv[i] = 1.0f / (row_index_end - row_index);
		}

		for (size_t i = 0; i < iterations; i++) {

			std::vector<std::vector<float>>& pr_read = i % 2 == 0 ? prv_1 : prv_2;
			std::vector<std::vector<float>>& pr_write = i % 2 == 1 ? prv_1 : prv_2;

#pragma omp parallel for
			for (size_t vertex = 0; vertex < this->row_ind.size(); vertex++) {
				uint32_t row_index = this->row_ind[vertex];
				const uint32_t row_index_end = vertex + 1 == this->row_ind.size() ? this->col_ind.size() : this->row_ind[vertex + 1];

				float page_rank_sum[num_dampening_factors];
				for (auto& v : page_rank_sum) {
					v = 0;
				}

				if (row_index_end - row_index >= 8) {
					__m256 page_rank_sum_v[num_dampening_factors];
					for (auto& v : page_rank_sum_v) {
						v = _mm256_setzero_ps();
					}

					/* For each neighbor in groups of 8 */
					for (uint32_t riev = row_index_end - 8; row_index < riev; row_index += 8) {
						const __m256i neighbor_v = _mm256_loadu_si256((const __m256i*)(this->col_ind.data() + row_index));
						const __m256 adjacency_v = _mm256_i32gather_ps(adjacent_vertices_inv.data(), neighbor_v, 1);

						for (size_t j = 0; j < num_dampening_factors; j++) {
							const __m256 page_rank_load_v = _mm256_i32gather_ps(pr_read[j].data(), neighbor_v, 1);
							page_rank_sum_v[j] = _mm256_fmadd_ps(page_rank_load_v, adjacency_v, page_rank_sum_v[j]);
						}
					}
					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] = InstructionUtils::sum_register(page_rank_sum_v[j]);
					}
				}

				/* For each remaining neighbor */
				for (; row_index < row_index_end; row_index++) {
					uint32_t neighbor = this->col_ind[row_index];
					float d = adjacent_vertices_inv[neighbor];

					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] += pr_read[j][neighbor] * d;
					}
				}

				for (size_t j = 0; j < num_dampening_factors; j++) {
					pr_write[j][vertex] = page_rank_sum[j] * dampening_factors[j] + dampening_probs[j];
				}
			}
		}

		return iterations % 2 == 1 ? prv_1 : prv_2;
	}

	void GraphCRS::breadth_first_traversal(uint32_t vertex) const {

		std::queue<uint32_t> frontier;
		frontier.push(vertex);

		std::vector<uint32_t> visited(this->num_vertices(), 0);

		uint32_t level = 1;

		while (!frontier.empty()) {

			size_t e = frontier.size();
#pragma omp parallel for
			for (size_t i = 0; i < e; i++) {

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
						visited[neighbor] = level;
#pragma omp critical
						{
							frontier.push(neighbor);
						}
					}
				}
			}

			level++;
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
