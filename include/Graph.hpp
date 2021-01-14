#pragma once

#include <vector>
#include <inttypes.h>
#include <string>

class Graph {

public:

	/**
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 * @return The edge's weight from vertex i to vertex j
	 */
	virtual const float weight(const uint32_t i, const uint32_t j) const = 0;

	/**
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 * @return A reference to the edge's weight from vertex i to vertex j
	 */
	virtual float& operator()(const uint32_t i, const uint32_t j) = 0;

	/**
	 * Sets the weight of the edge from vertex i to vertex j
	 *
	 * @param weight The weight for the edge
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 */
	virtual void set(const float weight, const uint32_t i, const uint32_t j) = 0;

	/**
	 * Saves the graph as a .csv or binary representation (.crs) based on
	 * path's file extention.
	 *
	 * @param path The path to save the file
	 */
	virtual void save(const std::string& path) const = 0;

	/**
	 * Converts the graph to a string in CVS format, where the first row are
	 * the values, the second row is the col_ind, and the third row is the
	 * row_ind
	 */
	virtual std::string to_string() const = 0;

	/**
	 * Prints the entire graph
	 */
	virtual void print() const = 0;

	/**
	 * Returns a vector of page rank values for all vertices
	 */
	virtual std::vector<float> page_rank(size_t iterations, float dampening_factor) const = 0;

	/**
	 * Performs a breadth first traversal starting from vertex
	 */
	virtual void breadth_first_traversal(uint32_t vertex) const = 0;

	/**
	 * Returns the number of edges in the graph
	 */
	virtual uint32_t num_edges() const = 0;

	/**
	 * Returns the number of vertices in the graph
	 */
	virtual uint32_t num_vertices() const = 0;

	/**
	 * Return the number of bytes the graph uses in memory
	 */
	virtual size_t byte_size() const = 0;
};