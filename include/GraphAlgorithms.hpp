#pragma once

#include <vector>
#include <queue>
#include "Benchmarks.hpp"
#include "GraphCRS.hpp"
#include <immintrin.h>
#include "InstructionUtils.hpp"
#include "omp.h"
#include "Bitmap.hpp"
#include "FormatUtils.hpp"
#include "BlockTimer.hpp"

namespace GraphAlgorithms {

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<float, temp_alloc_type<float>> page_rank_v_neighbors(const GraphCRS<alloc_type>& graph, size_t iterations, const std::vector<float> dampening_factors) {
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

#pragma omp parallel for schedule(dynamic,4)
			for (size_t vertex = 0; vertex < graph.num_vertices(); vertex++) {

				float page_rank_sum[num_dampening_factors];
				for (auto& v : page_rank_sum) {
					v = 0;
				}

				uint32_t row_index = graph.row_ind[vertex];
				uint32_t row_index_end = graph.row_ind[vertex + 1];

				if (row_index_end - row_index >= 8) {

					const __m256i ones_int_v = _mm256_set1_epi32(1);
					const __m256 ones_f_v = _mm256_set1_ps(1.0f);

					__m256 page_rank_sum_v[num_dampening_factors];
					for (auto& v : page_rank_sum_v) {
						v = _mm256_setzero_ps();
					}

					/* For each neighbor in groups of 8 */
					for (uint32_t riev = row_index_end - 8; row_index <= riev; row_index += 8) {

						__m256i neighbor_v = _mm256_loadu_si256((const __m256i*)(graph.col_ind + row_index));

						const __m256i neighbor_row_index_v = _mm256_i32gather_epi32((const int*)graph.row_ind, neighbor_v, 1);
						const __m256i neighbor_row_index_end_v = _mm256_i32gather_epi32((const int*)graph.row_ind, _mm256_add_epi32(neighbor_v, ones_int_v), 1);

						const __m256 adjacency_v = ones_f_v / _mm256_cvtepi32_ps(neighbor_row_index_end_v - neighbor_row_index_v);

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
					float d = 1.0f / graph.num_neighbors(neighbor);

					neighbor *= num_dampening_factors;
					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] += pr_read[neighbor + j] * d;
					}
				}

				/* Write the page rank */
				for (size_t j = 0; j < num_dampening_factors; j++) {
					pr_write[vertex * num_dampening_factors + j] = page_rank_sum[j] * dampening_factors[j] + dampening_probs[j];
				}
			}
		}
		return iterations % 2 == 1 ? prv_1 : prv_2;
	}

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

#pragma omp parallel for schedule(dynamic,4)
			for (size_t vertex = 0; vertex < graph.num_vertices(); vertex++) {

				float page_rank_sum[num_dampening_factors];
				for (auto& v : page_rank_sum) {
					v = 0;
				}

				uint32_t row_index = graph.row_ind[vertex];
				uint32_t row_index_end = graph.row_ind[vertex + 1];

				/* For each neighbor */
				for (; row_index < row_index_end; row_index++) {
					uint32_t neighbor = graph.col_ind[row_index];
					float d = 1.0f / graph.num_neighbors(neighbor);

					neighbor *= num_dampening_factors;

#pragma omp simd
					for (size_t j = 0; j < num_dampening_factors; j++) {
						page_rank_sum[j] += pr_read[neighbor + j] * d;
					}
				}

#pragma omp simd
				for (size_t j = 0; j < num_dampening_factors; j++) {
					pr_write[vertex * num_dampening_factors + j] = page_rank_sum[j] * dampening_factors[j] + dampening_probs[j];
				}
			}
		}

		return iterations % 2 == 1 ? prv_1 : prv_2;
	}

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<int32_t, temp_alloc_type<int32_t>> breadth_first_traversal_hybrid(const GraphCRS<alloc_type>& graph, uint32_t source_vertex) {

		std::vector<int32_t, temp_alloc_type<int32_t>> vertex_depth(graph.num_vertices(), -1);
		vertex_depth[source_vertex] = 0;

		/* Frontiers for Top to Bottom */
		std::vector<uint32_t, temp_alloc_type<uint32_t>> frontier_vec_1;
		frontier_vec_1.reserve(graph.num_vertices() / 2);
		frontier_vec_1.push_back(source_vertex);

		std::vector<uint32_t, temp_alloc_type<uint32_t>> frontier_vec_2;
		frontier_vec_2.reserve(graph.num_vertices() / 2);

		/* Frontiers for Bottom to Top */
		Bitmap<temp_alloc_type> frontier_bm_1(graph.num_vertices());
		frontier_bm_1.set_bit(source_vertex);
		Bitmap<temp_alloc_type> frontier_bm_2(graph.num_vertices());

		/* Create a write frontier vector for each thread */
		std::vector < std::vector<uint32_t, temp_alloc_type<uint32_t>>, temp_alloc_type<std::vector<uint32_t, temp_alloc_type<uint32_t>>>> local_write_vecs;
		local_write_vecs.resize(omp_get_max_threads());
		for (auto& vec : local_write_vecs) {
			vec.reserve(graph.num_vertices() / (2 * omp_get_max_threads()));
		}

		/* The depth from the source vertex */
		int32_t level = 1;

		/* The number of edges to check in the frontier */
		size_t m_f = graph.num_neighbors(source_vertex);
		/* The number of vertices in the frontier */
		size_t n_f = 1;
		/* The number of edges to check from unexplored vertices */
		size_t m_u = graph.num_edges();
		/* Top to bottom tuning parameter */
		double alpha = 14;
		/* Bottom to top tuning parameter */
		double beta = 24;

		bool top_to_bottom_state = true;

		while (n_f > 0) {

			std::vector<uint32_t, temp_alloc_type<uint32_t>>& frontier_vec_r = level % 2 == 1 ? frontier_vec_1 : frontier_vec_2;
			std::vector<uint32_t, temp_alloc_type<uint32_t>>& frontier_vec_w = level % 2 == 0 ? frontier_vec_1 : frontier_vec_2;

			Bitmap<temp_alloc_type>& frontier_bm_r = level % 2 == 1 ? frontier_bm_1 : frontier_bm_2;
			Bitmap<temp_alloc_type>& frontier_bm_w = level % 2 == 0 ? frontier_bm_1 : frontier_bm_2;

			if (top_to_bottom_state && (m_f > m_u / alpha)) {

				/* Convert the vec to bitmap (Top Down -> Bottom Up) */
				frontier_bm_r.clear();
				frontier_bm_w.clear();


#pragma omp parallel for schedule(static)
				for (size_t i = 0; i < frontier_vec_r.size(); i++) {
					frontier_bm_r.set_bit(frontier_vec_r[i]);
				}

				top_to_bottom_state = false;
			}
			else if (!top_to_bottom_state && (n_f < graph.num_vertices() / beta)) {

				/* Convert the bitmap to vec (Bottom Up -> Top Down) */

				/* Clear the bitmaps */
				frontier_vec_r.resize(0);
				frontier_vec_w.resize(0);

				/* Clear the local write vectors */
#pragma omp parallel for schedule(static,1)
				for (size_t i = 0; i < local_write_vecs.size(); i++) {
					local_write_vecs[i].resize(0);
				}

#pragma omp parallel for schedule(dynamic,4)
				for (size_t i = 0; i < frontier_bm_r.size(); i++) {
					uint64_t v = frontier_bm_r.get(i);
					const size_t num_bits = sizeof(uint64_t) * 8;
					const uint64_t mask = 1ul << (num_bits - 1);
					for (size_t j = 0; j < num_bits; j++) {
						if ((v & mask) > 0) {
							size_t vertex = i * num_bits + j;
							local_write_vecs[omp_get_thread_num()].push_back(vertex);
						}
						v = v << 1;
					}
				}

				/* Copy over the local write frontiers to the write frontier */
				frontier_vec_w.resize(n_f);

#pragma omp parallel
				{
					size_t i = 0;

					for (int j = 0; j < omp_get_thread_num(); j++) {
						i += local_write_vecs[j].size();
					}

					std::vector<uint32_t, temp_alloc_type<uint32_t>>& vec = local_write_vecs[omp_get_thread_num()];
					for (size_t j = 0; j < vec.size(); j++, i++) {
						frontier_vec_w[i] = vec[j];
					}
				}

				top_to_bottom_state = true;
			}

			size_t edges_checked = 0;
			n_f = 0;
			m_f = 0;

			/* Top Down BFS */
			if (top_to_bottom_state) {

				/* Clear the local write vectors */
#pragma omp parallel for schedule(static,1)
				for (size_t i = 0; i < local_write_vecs.size(); i++) {
					local_write_vecs[i].resize(0);
				}

				size_t fr_end = frontier_vec_r.size();
#pragma omp parallel for schedule(dynamic,4) reduction(+:edges_checked, n_f, m_f)
				for (size_t i = 0; i < fr_end; i++) {

					uint32_t vertex = frontier_vec_r[i];

					uint32_t row_index = graph.row_ind[vertex];
					uint32_t row_index_end = graph.row_ind[vertex + 1];

					/* For each neighbor */
					for (; row_index < row_index_end; row_index++) {
						uint32_t neighbor = graph.col_ind[row_index];
						if (vertex_depth[neighbor] == -1) {
							vertex_depth[neighbor] = level;
							n_f++;
							m_f += graph.num_neighbors(neighbor);
							local_write_vecs[omp_get_thread_num()].push_back(neighbor);
						}
					}

					edges_checked += row_index_end - row_index;
				}

				/* Copy over the local write frontiers to the write frontier */
				frontier_vec_w.resize(n_f);

#pragma omp parallel
				{
					size_t i = 0;

					for (int j = 0; j < omp_get_thread_num(); j++) {
						i += local_write_vecs[j].size();
					}

					std::vector<uint32_t, temp_alloc_type<uint32_t>>& vec = local_write_vecs[omp_get_thread_num()];
					for (size_t j = 0; j < vec.size(); j++, i++) {
						frontier_vec_w[i] = vec[j];
					}
				}

				/* Clear the read frontier */
				frontier_vec_r.resize(0);
			}
			else {

				/* Bottom Up BFS */
#pragma omp parallel for schedule(dynamic,4) reduction(+:edges_checked, n_f, m_f)
				for (size_t vertex = 0; vertex < graph.num_vertices(); vertex++) {
					if (vertex_depth[vertex] == -1) {

						uint32_t row_index = graph.row_ind[vertex];
						uint32_t row_index_end = graph.row_ind[vertex + 1];

						for (; row_index < row_index_end; row_index++) {
							uint32_t neighbor = graph.col_ind[row_index];
							if (frontier_bm_r.get_bit(neighbor) > 0) {
								frontier_bm_w.set_bit(vertex);
								vertex_depth[vertex] = vertex_depth[neighbor] + 1;
								n_f++;
								m_f += graph.num_neighbors(vertex);
								edges_checked += graph.num_neighbors(neighbor);
								break;
							}
						}
					}
				}

				frontier_bm_r.clear();
			}

			level++;
			m_u -= edges_checked;
		}

		return vertex_depth;
	}

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<int32_t, temp_alloc_type<int32_t>> breadth_first_traversal_top_down(const GraphCRS<alloc_type>& graph, uint32_t source_vertex) {

		std::vector<int32_t, temp_alloc_type<int32_t>> vertex_depth(graph.num_vertices(), -1);
		vertex_depth[source_vertex] = 0;

		/* Frontiers */
		std::vector<uint32_t, temp_alloc_type<uint32_t>> frontier_vec_1;
		frontier_vec_1.reserve(graph.num_vertices() / 2);
		frontier_vec_1.push_back(source_vertex);

		std::vector<uint32_t, temp_alloc_type<uint32_t>> frontier_vec_2;
		frontier_vec_2.reserve(graph.num_vertices() / 2);

		/* Create a write frontier vector for each thread */
		std::vector < std::vector<uint32_t, temp_alloc_type<uint32_t>>, temp_alloc_type<std::vector<uint32_t, temp_alloc_type<uint32_t>>>> local_write_vecs;
		local_write_vecs.resize(omp_get_max_threads());
		for (auto& vec : local_write_vecs) {
			vec.reserve(graph.num_vertices() / (2 * omp_get_max_threads()));
		}

		/* The depth from the source vertex */
		int32_t level = 1;
		/* The number of vertices in the frontier */
		size_t n_f = 1;

		while (n_f > 0) {

			BlockTimer timer("Top Down Step");

			std::vector<uint32_t, temp_alloc_type<uint32_t>>& frontier_vec_r = level % 2 == 1 ? frontier_vec_1 : frontier_vec_2;
			std::vector<uint32_t, temp_alloc_type<uint32_t>>& frontier_vec_w = level % 2 == 0 ? frontier_vec_1 : frontier_vec_2;

			/* Clear the local write vectors */
#pragma omp parallel for schedule(static,1)
			for (size_t i = 0; i < local_write_vecs.size(); i++) {
				local_write_vecs[i].resize(0);
			}

			n_f = 0;

			size_t fr_end = frontier_vec_r.size();
#pragma omp parallel for schedule(dynamic,4) reduction(+:n_f) 
			for (size_t i = 0; i < fr_end; i++) {

				uint32_t vertex = frontier_vec_r[i];

				uint32_t row_index = graph.row_ind[vertex];
				uint32_t row_index_end = graph.row_ind[vertex + 1];

				/* For each neighbor */
				for (; row_index < row_index_end; row_index++) {
					uint32_t neighbor = graph.col_ind[row_index];
					if (vertex_depth[neighbor] == -1) {
						vertex_depth[neighbor] = level;
						n_f++;
						local_write_vecs[omp_get_thread_num()].push_back(neighbor);
					}
				}
			}

			/* Copy over the local write frontiers to the write frontier */
			frontier_vec_w.resize(n_f);

#pragma omp parallel
			{
				size_t i = 0;

				for (int j = 0; j < omp_get_thread_num(); j++) {
					i += local_write_vecs[j].size();
				}

				std::vector<uint32_t, temp_alloc_type<uint32_t>>& vec = local_write_vecs[omp_get_thread_num()];
				for (size_t j = 0; j < vec.size(); j++, i++) {
					frontier_vec_w[i] = vec[j];
				}
			}

			/* Clear the read frontier */
			frontier_vec_r.resize(0);

			level++;
		}

		return vertex_depth;
	}

	template<template<class> class alloc_type, template<class> class temp_alloc_type>
	std::vector<int32_t, temp_alloc_type<int32_t>> breadth_first_traversal_bottom_up(const GraphCRS<alloc_type>& graph, uint32_t source_vertex) {

		std::vector<int32_t, temp_alloc_type<int32_t>> vertex_depth(graph.num_vertices(), -1);
		vertex_depth[source_vertex] = 0;

		/* Frontiers for Bottom to Top */
		Bitmap<temp_alloc_type> frontier_bm_1(graph.num_vertices());
		frontier_bm_1.set_bit(source_vertex);
		Bitmap<temp_alloc_type> frontier_bm_2(graph.num_vertices());

		std::vector<int8_t, temp_alloc_type<int8_t>> frontier_vec_1(graph.num_vertices(), 0);
		frontier_vec_1[source_vertex] = 1;
		std::vector<int8_t, temp_alloc_type<int8_t>> frontier_vec_2(graph.num_vertices(), 0);

		/* The depth from the source vertex */
		int32_t level = 1;
		/* The number of vertices in the frontier */
		size_t n_f = 1;

		while (n_f > 0) {

			BlockTimer timer("Bottom Up Step");

			Bitmap<temp_alloc_type>& frontier_bm_r = level % 2 == 1 ? frontier_bm_1 : frontier_bm_2;
			Bitmap<temp_alloc_type>& frontier_bm_w = level % 2 == 0 ? frontier_bm_1 : frontier_bm_2;

			std::vector<int8_t, temp_alloc_type<int8_t>>& frontier_vec_r = level % 2 == 1 ? frontier_vec_1 : frontier_vec_2;
			std::vector<int8_t, temp_alloc_type<int8_t>>& frontier_vec_w = level % 2 == 0 ? frontier_vec_1 : frontier_vec_2;

			n_f = 0;

			/* Bottom Up BFS */
#pragma omp parallel for schedule(dynamic,16) reduction(+:n_f)
			for (size_t vertex = 0; vertex < graph.num_vertices(); vertex++) {
				if (vertex_depth[vertex] == -1) {

					uint32_t row_index = graph.row_ind[vertex];
					uint32_t row_index_end = graph.row_ind[vertex + 1];

					for (; row_index < row_index_end; row_index++) {
						uint32_t neighbor = graph.col_ind[row_index];

						if (frontier_vec_r[neighbor] == 1) {
							frontier_vec_w[vertex] = 1;
							vertex_depth[vertex] = level;
							n_f++;
							break;
						}

						/*
						if (frontier_bm_r.get_bit(neighbor) == 1) {
							frontier_bm_w.set_bit(vertex);
							vertex_depth[vertex] = level;
							n_f++;
							break;
						}
						*/
					}
				}
			}

			//frontier_bm_r.clear();

#pragma omp parallel for schedule(static)
			for (size_t i = 0; i < frontier_vec_r.size(); i++) {
				frontier_vec_r[i] = 0;
			}

			level++;
		}

		return vertex_depth;
	}
}