#pragma once

#include <vector>
#include <queue>
#include "Benchmarks.hpp"
#include "GraphCRS.hpp"
#include <immintrin.h>
#include "InstructionUtils.hpp"
#include "omp.h"
#include "Bitmap.hpp"

namespace GraphAlgorithms {

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<float, temp_alloc_type<float>> page_rank(const GraphCRS<alloc_type>& graph, size_t iterations, const std::vector<float> dampening_factors) {

		/* Initialize dampening factors */
		const size_t num_dampening_factors = dampening_factors.size();
		const float init_prob = 1.0f / graph.num_vertices();
		std::vector<float, temp_alloc_type<float>> dampening_probs;
		for (auto& dampening_factor : dampening_factors) {
			dampening_probs.push_back((1.0f - dampening_factor) / graph.num_vertices());
		}

		/* Use two vectors since the next iteration relies on the current iteration */
		std::vector<float, temp_alloc_type<float>> prv_1(num_dampening_factors * graph.num_vertices(), init_prob);
		std::vector<float, temp_alloc_type<float>> prv_2(num_dampening_factors * graph.num_vertices(), init_prob);

		for (size_t i = 0; i < iterations; i++) {

			std::vector<float, temp_alloc_type<float>>& pr_read = i % 2 == 0 ? prv_1 : prv_2;
			std::vector<float, temp_alloc_type<float>>& pr_write = i % 2 == 1 ? prv_1 : prv_2;

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
						__m256i neighbor_v = _mm256_loadu_si256((const __m256i*)(graph.col_ind + row_index));
						uint32_t neighbor = graph.col_ind[row_index];
						uint32_t neighbor_row_index_end = neighbor + 8 >= graph.num_vertices() ? graph.num_edges() : graph.row_ind[neighbor + 8];

						/* Extract the 5th element, _mm256_slli_si256 shifts in 128b sections, so the 1st and 5th element are lost */
						uint32_t t = _mm256_extract_epi32(neighbor_v, 4);
						__m256i neighbor_shifted_v = _mm256_slli_si256(neighbor_v, 4);

						/* Fill in the missing elements to compute number of neighbors */
						_mm256_insert_epi32(neighbor_shifted_v, graph.col_ind[neighbor_row_index_end], 7);
						_mm256_insert_epi32(neighbor_shifted_v, t, 3);

						__m256 adjacency_v = _mm256_set_ps(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
						adjacency_v = adjacency_v / _mm256_cvtepi32_ps((neighbor_shifted_v - neighbor_v));

						/* Scale the neighbor vertex indexes for access into the page rank matrix */
						neighbor_v = _mm256_mullo_epi32(neighbor_v, _mm256_set1_epi32(num_dampening_factors));

						for (size_t j = 0; j < num_dampening_factors; j++) {
							const __m256 page_rank_load_v = _mm256_i32gather_ps(pr_read.data() + j, neighbor_v, 1);
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
					uint32_t neighbor_row_index = graph.row_ind[neighbor];
					uint32_t neighbor_row_index_end = neighbor + 1 == graph.num_vertices() ? graph.num_edges() : graph.row_ind[neighbor + 1];
					float d = 1.0f / (neighbor_row_index_end - neighbor_row_index);

					neighbor *= num_dampening_factors;
					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] += pr_read[neighbor + j] * d;
					}
				}

				for (size_t j = 0; j < num_dampening_factors; j++) {
					pr_write[vertex * num_dampening_factors + j] = page_rank_sum[j] * dampening_factors[j] + dampening_probs[j];
				}
			}
		}

		return iterations % 2 == 1 ? prv_1 : prv_2;
	}

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<int32_t, temp_alloc_type<int32_t>> breadth_first_traversal(const GraphCRS<alloc_type>& graph, uint32_t source_vertex) {

		std::vector<uint32_t, temp_alloc_type<uint32_t>> frontier_1;
		frontier_1.reserve(graph.num_vertices() / 2);
		frontier_1.push_back(source_vertex);

		std::vector<uint32_t, temp_alloc_type<uint32_t>> frontier_2;
		frontier_2.reserve(graph.num_vertices() / 2);

		std::vector<uint32_t, temp_alloc_type<uint32_t>> visited(graph.num_vertices(), 0);

		uint32_t level = 1;

		while (!frontier_1.empty() || !frontier_2.empty()) {

			std::vector<uint32_t, temp_alloc_type<uint32_t>>& frontier_r = level % 2 == 1 ? frontier_1 : frontier_2;
			std::vector<uint32_t, temp_alloc_type<uint32_t>>& frontier_w = level % 2 == 0 ? frontier_1 : frontier_2;

			size_t fr_end = frontier_r.size();
#pragma omp parallel for schedule(dynamic,4)
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

		return visited;
	}

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<int32_t, temp_alloc_type<int32_t>> breadth_first_traversal_hybrid(const GraphCRS<alloc_type>& graph, uint32_t source_vertex) {

		std::vector<int32_t, temp_alloc_type<int32_t>> visited(graph.num_vertices(), -1);

		Bitmap<temp_alloc_type> frontier_1;
		frontier_1.resize(graph.num_vertices());
		frontier_1.set_bit(source_vertex);

		Bitmap<temp_alloc_type> frontier_2;
		frontier_2.resize(graph.num_vertices());

		uint32_t level = 1;
		bool frontier_empty = false;

		/* The number of edges to check in the frontier */
		size_t m_f = 0;
		/* The number of vertices in the frontier */
		size_t n_f = 0;
		/* The number of edges to check from unexplored vertices */
		size_t m_u = graph.num_edges();
		/* Top to bottom tuning parameter */
		double alpha = 14;
		/* Bottom to top tuning parameter */
		double beta = 24;

		bool top_to_bottom_state = true;

		while (!frontier_empty) {

			frontier_empty = true;
			Bitmap<temp_alloc_type>& frontier_r = level % 2 == 1 ? frontier_1 : frontier_2;
			Bitmap<temp_alloc_type>& frontier_w = level % 2 == 0 ? frontier_1 : frontier_2;

			const size_t fr_end = frontier_r.size();

			/* Count the number of edges in the frontier to check and the number of vertices in the frontier */
#pragma omp parallel for schedule(static) reduction(+:m_f) reduction(+:n_f)
			for (size_t i = 0; i < fr_end; i++) {
				uint64_t v = frontier_r.get(i);
				const size_t num_bits = sizeof(uint64_t) * 8;
				const uint64_t mask = 1ul << (num_bits - 1);
				for (size_t j = 0; j < num_bits; j++) {
					if ((v & mask) > 0) {
						size_t vertex = i * num_bits + j;
						uint32_t row_index = graph.row_ind[vertex];
						uint32_t row_index_end = (vertex + 1 == graph.num_vertices()) ? graph.num_edges() : graph.row_ind[vertex + 1];
						m_f += row_index_end - row_index;
						n_f++;
					}
					v = v << 1;
				}
			}

			if (top_to_bottom_state && (m_f > double(m_u) / alpha)) {
				top_to_bottom_state = false;
			}
			else if (!top_to_bottom_state && (n_f < graph.num_vertices() / beta)) {
				top_to_bottom_state = true;
			}

			size_t edges_checked = 0;

			/* Top Down BFS */
			if (top_to_bottom_state) {
#pragma omp parallel for schedule(dynamic,4) reduction(+:edges_checked)
				for (size_t i = 0; i < fr_end; i++) {

					int64_t offset;
					while ((offset = frontier_r.get_next(i)) >= 0) {
						uint32_t vertex = i * sizeof(int64_t) + offset;
						uint32_t row_index_end = (vertex + 1 == graph.num_vertices()) ? graph.num_edges() : graph.row_ind[vertex + 1];

						/* For each neighbor */
						for (uint32_t row_index = graph.row_ind[vertex]; row_index < row_index_end; row_index++) {
							uint32_t neighbor = graph.col_ind[row_index];
							if (visited[neighbor] == -1) {
								visited[neighbor] = level;
								frontier_w.set_bit(neighbor);
								frontier_empty = false;
							}
						}

						edges_checked += row_index_end - graph.row_ind[vertex];
					}
				}
			}
			else {
				/* Bottom Up BFS */
#pragma omp parallel for schedule(dynamic,4) reduction(+:edges_checked)
				for (size_t vertex = 0; vertex < graph.num_vertices(); vertex++) {
					if (visited[vertex] == -1) {

						uint32_t row_index_end = (vertex + 1 == graph.num_vertices()) ? graph.num_edges() : graph.row_ind[vertex + 1];

						for (uint32_t row_index = graph.row_ind[vertex]; row_index < row_index_end; row_index++) {
							uint32_t neighbor = graph.col_ind[row_index];
							if (frontier_r.get_bit(neighbor) > 0) {
								visited[vertex] = visited[neighbor] + 1;
								frontier_w.set_bit(neighbor);
								frontier_empty = false;
							}
						}

						edges_checked += row_index_end - graph.row_ind[vertex];
					}
				}
			}

			level++;
			m_u -= edges_checked;
		}

		return visited;
	}
}