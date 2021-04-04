#pragma once

#include <inttypes.h>
#include <cstddef>
#include <tuple>

template<template<class> class T>
class GraphCRS {
public:

    GraphCRS() : GraphCRS::GraphCRS(0, 0) {}

    GraphCRS(uint32_t num_vertices, uint32_t num_edges) :
        val(nullptr),
        col_ind(nullptr),
        row_ind(nullptr),
        m_num_vertices(num_vertices),
        m_num_edges(num_edges),
        m_float_alloc(T<float>()),
        m_uint32_alloc(T<uint32_t>()) {
        this->val = this->m_float_alloc.allocate(num_edges);
        this->col_ind = this->m_uint32_alloc.allocate(num_edges);
        this->row_ind = this->m_uint32_alloc.allocate(num_vertices + 1);
        this->row_ind[this->m_num_vertices] = this->m_num_edges;
    }

    void free() {
        this->m_float_alloc.deallocate(this->val, this->m_num_edges);
        this->val = nullptr;
        this->m_uint32_alloc.deallocate(this->col_ind, this->m_num_edges);
        this->col_ind = nullptr;
        this->m_uint32_alloc.deallocate(this->row_ind, this->m_num_vertices + 1);
        this->row_ind = nullptr;
    }

    size_t num_edges() const { return this->m_num_edges; }
    size_t num_vertices() const { return this->m_num_vertices; }

    size_t num_neighbors(uint32_t vertex) const {
        std::pair<uint32_t, uint32_t> row_indices = this->row_indices(vertex);
        return std::get<1>(row_indices) - std::get<0>(row_indices);
    }

    std::pair<uint32_t, uint32_t> row_indices(uint32_t vertex) const {
        uint32_t row_index = this->row_ind[vertex];
        uint32_t row_index_end = (vertex + 1 == this->num_vertices()) ? this->num_edges() : this->row_ind[vertex + 1];
        return std::make_pair(row_index, row_index_end);
    }

    size_t byte_size() const {
        return sizeof(float) * this->m_num_edges + sizeof(uint32_t) * (this->m_num_vertices + this->m_num_edges);
    }

public:
    float* val;
    uint32_t* col_ind;
    uint32_t* row_ind;

private:
    size_t m_num_vertices;
    size_t m_num_edges;
    T<float> m_float_alloc;
    T<uint32_t> m_uint32_alloc;
};