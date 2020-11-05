#include "GraphCRS.hpp"

#include <iostream>
#include <fstream>

GraphCRS::GraphCRS(std::vector<float> val,
	std::vector<uint32_t> col_ind,
	std::vector<uint32_t> row_ind) : val(val),
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

	std::string output = this->to_string();

	std::ofstream file(path, std::ios::binary);

	if (file) {
		file.write(output.c_str(), output.size());
		file.close();
	}
}

void GraphCRS::load(std::string path) {
	std::ifstream file(path, std::ios::binary);

	if (file) {
		file.seekg(0, std::ios::end);
		std::string input;
		input.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&input[0], input.size());
		file.close();
	}
	else {
		printf("Unable to load GraphCRS from %s\n", path.c_str());
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
