#pragma once

#include <functional>

template<class T>
class Graph {
public:
	const uint32_t numOfNodes;

	Graph(const uint32_t numOfNodes, T* arr) :
		numOfNodes(numOfNodes),
		arr(arr) {}


	const T weight(const uint32_t i, const uint32_t j) {
		return this->arr[this->index(i, j)];
	}

	T& operator()(const uint32_t i, const uint32_t j) {
		return this->arr[this->index(i, j)];
	}

	void set(const T weight, const uint32_t i, const uint32_t j) {
		this->arr[this->index(i, j)] = weight;
	}

	void forEach(std::function<void(T& v, const uint32_t i, const uint32_t j)> func) {
		for (uint32_t i = 0; i < this->numOfNodes; i++) {
			for (uint32_t j = 0; j < this->numOfNodes; j++) {
				func(this->arr[this->index(i, j)], i, j);
			}
		}
	}

private:
	T* arr;

	const uint32_t index(const uint32_t i, const uint32_t j) {
		return this->numOfNodes * i + j;
	}
};
