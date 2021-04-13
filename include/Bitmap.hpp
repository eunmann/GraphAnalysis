#pragma once

#include <vector>
#include <inttypes.h>
#include <stdio.h>
#include <atomic>

template<template<typename> typename alloc_type>
class Bitmap {

public:

	Bitmap(size_t size) :
		m_vec((size + sizeof(uint64_t) * 8) / sizeof(uint64_t) * 8) {
	}

	void set_bit(size_t bit_index) {
		const size_t num_bits = sizeof(uint64_t) * 8;
		const uint64_t mask = 1ul << (num_bits - 1);

		size_t vec_index = bit_index / num_bits;
		size_t offset = bit_index % num_bits;

		this->m_vec[vec_index] |= mask >> offset;
	}

	uint64_t get_bit(size_t bit_index) {
		const size_t num_bits = sizeof(uint64_t) * 8;

		size_t vec_index = bit_index / num_bits;
		size_t offset = bit_index % num_bits;

		return (this->m_vec[vec_index] >> (num_bits - 1 - offset)) & 1ul;
	}

	void clear() {
#pragma omp parallel for schedule(static)
		for (size_t i = 0; i < this->m_vec.size(); i++) {
			this->m_vec[i] = 0;
		}
	}

	uint64_t get(size_t index) {
		return this->m_vec[index];
	}

	size_t size() {
		return this->m_vec.size();
	}

private:
	std::vector<std::atomic_uint64_t, alloc_type<std::atomic_uint64_t>> m_vec;
};