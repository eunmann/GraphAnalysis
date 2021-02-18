#pragma once

#include "Graph.hpp"
#include "PMEM/vector.hpp"

namespace PMEM {

	/**
	 * Compressed Row Storage graph implementation
	 */
	class GraphCRS : public Graph {
	public:

		GraphCRS(std::string directory);

		GraphCRS(PMEM::vector<float> val,
			PMEM::vector<uint32_t> col_ind,
			PMEM::vector<uint32_t> row_ind);

		float weight(const uint32_t i, const uint32_t j) const;

		float& operator()(const uint32_t i, const uint32_t j);

		void set(const float weight, const uint32_t i, const uint32_t j);

		void save(const std::string& path) const;

		std::string to_string() const;

		void print() const;

		std::vector<std::vector<float>> page_rank(size_t iterations, const std::vector<float> dampening_factors) const;

		void breadth_first_traversal(uint32_t vertex) const;

		uint32_t num_edges() const;

		uint32_t num_vertices() const;

		size_t byte_size() const;

		/**
		 * Returns if there underlying memory is persistent
		 */
		bool is_pmem() const;

		/**
		 * Free the graph from PMEM
		 */
		void free();

	private:
		PMEM::vector<float> val;
		PMEM::vector<uint32_t> col_ind;
		PMEM::vector<uint32_t> row_ind;

		/**
		 * Transforms a 2 dimensional index into arr into a single dimensinal index
		 *
		 * @param i The row index
		 * @param j The column index
		 * @return The index into val
		 */
		uint32_t index(const uint32_t i, const uint32_t j) const;
	};
}