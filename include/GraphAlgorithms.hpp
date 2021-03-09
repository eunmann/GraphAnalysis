#pragma once

#include <vector>
#include <queue>
#include "Benchmarks.hpp"
#include "GraphCRS.hpp"
#include <immintrin.h>
#include "InstructionUtils.hpp"
#include "omp.h"

namespace GraphAlgorithms {

	template<template<class> class T>
	std::vector<std::vector<float>> page_rank(const GraphCRS<T>& graph, size_t iterations, const std::vector<float> dampening_factors) {

		const size_t num_dampening_factors = dampening_factors.size();
		const float init_prob = 1.0f / graph.num_vertices();
		std::vector<float> dampening_probs;
		for (auto& dampening_factor : dampening_factors) {
			dampening_probs.push_back((1.0f - dampening_factor) / graph.num_vertices());
		}

		/* Use two vectors since the next iteration relies on the current iteration */
		std::vector<std::vector<float>> prv_1;
		std::vector<std::vector<float>> prv_2;
		for (size_t i = 0; i < num_dampening_factors; i++) {
			prv_1.push_back(std::vector<float>(graph.num_vertices(), init_prob));
			prv_2.push_back(std::vector<float>(graph.num_vertices(), init_prob));
		}

		/* Compute the number of adjacent vertices inverse for each vertex */
		std::vector<float> adjacent_vertices_inv(graph.num_vertices());
		const size_t e = adjacent_vertices_inv.size();
#pragma omp parallel for schedule(static)
		for (size_t i = 0; i < e; i++) {
			uint32_t row_index = graph.row_ind[i];
			uint32_t row_index_end = i + 1 == graph.num_vertices() ? graph.num_edges() : graph.row_ind[i + 1];
			adjacent_vertices_inv[i] = 1.0f / (row_index_end - row_index);
		}

		for (size_t i = 0; i < iterations; i++) {

			std::vector<std::vector<float>>& pr_read = i % 2 == 0 ? prv_1 : prv_2;
			std::vector<std::vector<float>>& pr_write = i % 2 == 1 ? prv_1 : prv_2;

#pragma omp parallel for schedule(static)
			for (size_t vertex = 0; vertex < graph.num_vertices(); vertex++) {
				uint32_t row_index = graph.row_ind[vertex];
				const uint32_t row_index_end = vertex + 1 == graph.num_vertices() ? graph.num_edges() : graph.row_ind[vertex + 1];

				float page_rank_sum[num_dampening_factors];
				for (auto& v : page_rank_sum) {
					v = 0;
				}

				if (row_index_end - row_index >= 8) {
					__m256 page_rank_sum_v[num_dampening_factors];
					for (auto& v : page_rank_sum_v) {
						v = _mm256_setzero_ps();
					}

					/* For each neighbor in groups of 8 */
					for (uint32_t riev = row_index_end - 8; row_index < riev; row_index += 8) {
						const __m256i neighbor_v = _mm256_loadu_si256((const __m256i*)(graph.col_ind + row_index));
						const __m256 adjacency_v = _mm256_i32gather_ps(adjacent_vertices_inv.data(), neighbor_v, 1);

						for (size_t j = 0; j < num_dampening_factors; j++) {
							const __m256 page_rank_load_v = _mm256_i32gather_ps(pr_read[j].data(), neighbor_v, 1);
							page_rank_sum_v[j] = _mm256_fmadd_ps(page_rank_load_v, adjacency_v, page_rank_sum_v[j]);
						}
					}
					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] = InstructionUtils::sum_register(page_rank_sum_v[j]);
					}
				}

				/* For each remaining neighbor */
				for (; row_index < row_index_end; row_index++) {
					uint32_t neighbor = graph.col_ind[row_index];
					float d = adjacent_vertices_inv[neighbor];

					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] += pr_read[j][neighbor] * d;
					}
				}

				for (size_t j = 0; j < num_dampening_factors; j++) {
					pr_write[j][vertex] = page_rank_sum[j] * dampening_factors[j] + dampening_probs[j];
				}
			}
		}

		return iterations % 2 == 1 ? prv_1 : prv_2;
	}

	template<template<class> class T>
	void breadth_first_traversal(const GraphCRS<T>& graph, uint32_t vertex) {

		std::vector<uint32_t> frontier_vec_1;
		frontier_vec_1.reserve(graph.num_vertices() / 2);
		frontier_vec_1.push_back(vertex);

		std::vector<uint32_t> frontier_vec_2;
		frontier_vec_2.reserve(graph.num_vertices() / 2);

		std::vector<uint32_t> visited(graph.num_vertices(), 0);

		uint32_t level = 1;

		while (!frontier_vec_1.empty() || !frontier_vec_2.empty()) {

			std::vector<uint32_t>& frontier_r = level % 2 == 1 ? frontier_vec_1 : frontier_vec_2;
			std::vector<uint32_t>& frontier_w = level % 2 == 0 ? frontier_vec_1 : frontier_vec_2;

			size_t fr_end = frontier_r.size();
#pragma omp parallel for schedule(static)
			for (size_t i = 0; i < fr_end; i++) {

				uint32_t vertex = frontier_r[i];
				uint32_t row_index_end = (vertex + 1 == graph.num_vertices()) ? graph.num_edges() : graph.row_ind[vertex + 1];

				/* For each neighbor */
				for (uint32_t row_index = graph.row_ind[vertex]; row_index < row_index_end; row_index++) {
					uint32_t neighbor = graph.col_ind[row_index];
					if (visited[neighbor] == 0) {
						visited[neighbor] = level;
#pragma omp critical
						{
							frontier_w.push_back(neighbor);
						}
					}
				}
			}

			level++;
			frontier_r.resize(0);
		}
	}
}