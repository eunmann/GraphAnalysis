#pragma once

#include <vector>
#include <inttypes.h>
#include <stdio.h>

template<template<typename> typename alloc_type>
class Bitmap {

public:

	void resize(size_t size) {
		const size_t num_bits = sizeof(uint64_t) * 8;
		size_t alloc_size = (size + num_bits) / num_bits;
		this->m_vec.resize(alloc_size);
	}

	void set_bit(size_t bit_index) {
		const size_t num_bits = sizeof(uint64_t) * 8;
		const uint64_t mask = 1ul << (num_bits - 1);

		size_t vec_index = bit_index / num_bits;
		size_t offset = bit_index % num_bits;

		this->m_vec[vec_index] |= mask >> offset;
	}

	void unset_bit(size_t bit_index) {
		const size_t num_bits = sizeof(uint64_t) * 8;
		const uint64_t mask = 1ul << (num_bits - 1);

		size_t vec_index = bit_index / num_bits;
		size_t offset = bit_index % num_bits;

		this->m_vec[vec_index] &= ~(mask >> offset);
	}

	uint64_t get_bit(size_t bit_index) {
		const size_t num_bits = sizeof(uint64_t) * 8;

		size_t vec_index = bit_index / num_bits;
		size_t offset = bit_index % num_bits;

		return (this->m_vec[vec_index] >> (num_bits - 1 - offset)) & 1ul;
	}

	int64_t get_next(size_t index) {

		const size_t num_bits = sizeof(uint64_t) * 8;
		const uint64_t mask = 1ul << (num_bits - 1);
		int64_t offset = 0;
		uint64_t value = this->m_vec[index];

		for (size_t i = 0; i < num_bits; i++) {
			if ((value & mask) > 0) {
				this->m_vec[index] &= ~(mask >> offset);
				return offset;
			}
			else {
				value = value << 1;
				offset++;
			}
		}

		return -1;
	}

	uint64_t get(size_t index) {
		return this->m_vec[index];
	}

	size_t size() {
		return this->m_vec.size();
	}

private:
	std::vector<uint64_t, alloc_type<uint64_t>> m_vec;
};