#pragma once

#include <inttypes.h>
#include <PMEM/vector.hpp>
#include <functional>
#include <string>
#include <vector>

namespace PMEM {

	/**
	 * Compressed Row Storage graph implementation
	 */
	class GraphCRS {
	public:

		GraphCRS(PMEM::vector<float> val,
			PMEM::vector<uint32_t> col_ind,
			PMEM::vector<uint32_t> row_ind);

		/**
		 * @param i Index of the start vertex
		 * @param j Index of the destination vertex
		 * @return The edge's weight from vertex i to vertex j
		 */
		const float weight(const uint32_t i, const uint32_t j);

		/**
		 * @param i Index of the start vertex
		 * @param j Index of the destination vertex
		 * @return A reference to the edge's weight from vertex i to vertex j
		 */
		float& operator()(const uint32_t i, const uint32_t j);

		/**
		 * Sets the weight of the edge from vertex i to vertex j
		 *
		 * @param weight The weight for the edge
		 * @param i Index of the start vertex
		 * @param j Index of the destination vertex
		 */
		void set(const float weight, const uint32_t i, const uint32_t j);

		/**
		 * Applies the input function to each vertex in the graph.
		 *
		 * @param func The function to execute, it takes a reference to the weight, the starting
		 * vertex i and the destination vertex j as parameters.
		 */
		void for_each(std::function<void(float& v, const uint32_t i, const uint32_t j)> func);

		/**
		 * Saves the graph as a .csv or binary representation (.crs) based on
		 * path's file extention.
		 *
		 * @param path The path to save the file
		 */
		void save(std::string path);


		/**
		 * Converts the graph to a string in CVS format, where the firt row are
		 * the values, the second row is the col_ind, and the third row is the
		 * row_ind
		 */
		std::string to_string();

		/**
		 * Prints the entire graph
		 */
		void print();

		/**
		 * Free memory
		 */
		void free();

		/**
		 * Returns the path of vertices that are on the shortest path from node
		 * source to node destination
		 */
		std::vector<uint32_t> shortest_path(uint32_t source, uint32_t destination);

		/**
		 * Returns a vector of page rank values for all vertices
		 */
		std::vector<float> page_rank(size_t iterations, float dampening_factor);

		/**
		 * Performs a breadth first traversal starting from vertex
		 */
		void breadth_first_traversal(uint32_t vertex);

		/**
		 * Returns the number of edges in the graph
		 */
		uint32_t num_edges();

		/**
		 * Returns the number of vertices in the graph
		 */
		uint32_t num_vertices();

		/**
		 * Return the number of bytes the graph uses in memory
		 */
		size_t byte_size();
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
		const uint32_t index(const uint32_t i, const uint32_t j);
	};
}