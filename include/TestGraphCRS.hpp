#pragma once

#include <inttypes.h>

template<template<class> class T>
class TestGraphCRS {
public:
	TestGraphCRS(uint32_t num_vertices, uint32_t num_edges) :
		m_num_vertices(num_vertices),
		m_num_edges(num_edges),
		m_float_alloc(T<float>()),
		m_uint32_alloc(T<uint32_t>()) {
		this->m_val = this->m_float_alloc.allocate(num_edges);
		this->m_col_ind = this->m_uint32_alloc.allocate(num_edges);
		this->m_row_ind = this->m_uint32_alloc.allocate(num_vertices);
	}

	~TestGraphCRS() {
		this->m_float_alloc.deallocate(this->m_val, this->m_num_edges);
		this->m_val = nullptr;
		this->m_uint32_alloc.deallocate(this->m_col_ind, this->m_num_edges);
		this->m_col_ind = nullptr;
		this->m_uint32_alloc.deallocate(this->m_row_ind, this->m_num_vertices);
		this->m_row_ind = nullptr;
	}

	uint32_t num_vertices() const { return this->m_num_vertices; }
	uint32_t num_edges() const { return this->m_num_edges; }
	float* val() const { return this->m_val; }
	uint32_t* col_ind() const { return this->m_col_ind; }
	uint32_t* row_ind() const { return this->m_row_ind; }
	size_t byte_size() const {
		return sizeof(float) * this->m_num_edges + sizeof(uint32_t) * (this->m_num_vertices + this->m_num_edges);
	}

private:
	float* m_val;
	uint32_t* m_col_ind;
	uint32_t* m_row_ind;
	uint32_t m_num_vertices;
	uint32_t m_num_edges;
	T<float> m_float_alloc;
	T<uint32_t> m_uint32_alloc;
};