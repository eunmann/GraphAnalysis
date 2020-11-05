#pragma once

#include <functional>

/**
 * A template class to represent a graph. The memory is considered square, NxN, where N is the number
 * of vertices in the graph.
 */
template<class T>
class Graph {
public:
	const uint32_t numVertices;

	/**
	 * @param numVertices The number of vertices the graph contains
	 * @param arr A pointer to memory for the object to reference. It should be square.
	 */
	Graph(const uint32_t numVertices, T* arr) :
		numVertices(numVertices),
		arr(arr) {}

	/**
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 * @return The edge's weight from vertex i to vertex j
	 */
	const T weight(const uint32_t i, const uint32_t j) {
		return this->arr[this->index(i, j)];
	}

	/**
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 * @return A reference to the edge's weight from vertex i to vertex j
	 */
	T& operator()(const uint32_t i, const uint32_t j) {
		return this->arr[this->index(i, j)];
	}

	/**
	 * Sets the weight of the edge from vertex i to vertex j
	 *
	 * @param weight The weight for the edge
	 * @param i Index of the start vertex
	 * @param j Index of the destination vertex
	 */
	void set(const T weight, const uint32_t i, const uint32_t j) {
		this->arr[this->index(i, j)] = weight;
	}

	/**
	 * Applies the input function to each vertex in the graph.
	 *
	 * @param func The function to execute, it takes a reference to the weight, the starting
	 * vertex i and the destination vertex j as parameters.
	 */
	void for_each(std::function<void(T& v, const uint32_t i, const uint32_t j)> func) {
		for (uint32_t i = 0; i < this->numVertices; i++) {
			for (uint32_t j = 0; j < this->numVertices; j++) {
				func((*this)(i, j), i, j);
			}
		}
	}

private:

	/**
	 * The pointer into memory
	 */
	T* arr;

	/**
	 * Transforms a 2 dimensional index into arr into a single dimensinal index
	 *
	 * @param i The row index
	 * @param j The column index
	 * @return The index into arr
	 */
	const uint32_t index(const uint32_t i, const uint32_t j) {
		return this->numVertices * i + j;
	}
};
