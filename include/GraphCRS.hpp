#pragma once

#include <inttypes.h>
#include <cstddef>

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
		this->row_ind = this->m_uint32_alloc.allocate(num_vertices);
	}

	void free() {
		this->m_float_alloc.deallocate(this->val, this->m_num_edges);
		this->val = nullptr;
		this->m_uint32_alloc.deallocate(this->col_ind, this->m_num_edges);
		this->col_ind = nullptr;
		this->m_uint32_alloc.deallocate(this->row_ind, this->m_num_vertices);
		this->row_ind = nullptr;
	}

	size_t num_edges() const { return this->m_num_edges; }
	size_t num_vertices() const { return this->m_num_vertices; }

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