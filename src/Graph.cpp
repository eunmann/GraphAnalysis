#include "Graph.hpp"

Graph::Graph(const uint32_t numOfNodes) : numOfNodes(numOfNodes) {
    this->arr = new uint32_t[numOfNodes * numOfNodes];
}

const uint32_t Graph::weight(const uint32_t i, const uint32_t j) {
    return this->arr[this->index(i, j)];
}

uint32_t& Graph::operator()(const uint32_t i, const uint32_t j) {
    return this->arr[this->index(i, j)];
}

void Graph::set(const uint32_t weight, const uint32_t i, const uint32_t j) {
    this->arr[this->index(i, j)] = weight;
}
const uint32_t Graph::index(const uint32_t i, const uint32_t j) {
    return this->numOfNodes * i + j;
}

void Graph::forEach(std::function<void(uint32_t& v, const uint32_t i, const uint32_t j)> func) {
    for (uint32_t i = 0; i < this->numOfNodes; i++) {
        for (uint32_t j = 0; j < this->numOfNodes; j++) {
            func(this->arr[this->index(i, j)], i, j);
        }
    }
}