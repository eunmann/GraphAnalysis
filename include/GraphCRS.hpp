#pragma once

#include <inttypes.h>
#include <vector>
#include <functional>
#include <string>

/**
 * Compressed Row Storage graph implementation
 */
class GraphCRS {
public:

	GraphCRS();

	GraphCRS(std::vector<float> val,
		std::vector<uint32_t> col_ind,
		std::vector<uint32_t> row_ind);

	/**
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 * @return The edge's weight from vertex i to vertex j
	 */
	const float weight(const uint32_t i, const uint32_t j) const;

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
	void save(const std::string& path) const;

	/**
	 * Converts the graph to a string in CVS format, where the first row are
	 * the values, the second row is the col_ind, and the third row is the
	 * row_ind
	 */
	std::string to_string() const;

	/**
	 * Prints the entire graph
	 */
	void print() const;

	/**
	 * Returns a vector of page rank values for all vertices
	 */
	std::vector<float> page_rank(size_t iterations, float dampening_factor) const;

	/**
	 * Performs a breadth first traversal starting from vertex
	 */
	void breadth_first_traversal(uint32_t vertex) const;

	/**
	 * Returns the number of edges in the graph
	 */
	uint32_t num_edges() const;

	/**
	 * Returns the number of vertices in the graph
	 */
	uint32_t num_vertices() const;

	/**
	 * Return the number of bytes the graph uses in memory
	 */
	size_t byte_size() const;

private:
	std::vector<float> val;
	std::vector<uint32_t> col_ind;
	std::vector<uint32_t> row_ind;

	/**
	 * Transforms a 2 dimensional index into arr into a single dimensinal index
	 *
	 * @param i The row index
	 * @param j The column index
	 * @return The index into val
	 */
	const uint32_t index(const uint32_t i, const uint32_t j) const;
};