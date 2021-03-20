#include "InstructionUtils.hpp"

namespace InstructionUtils {
	float sum_register(__m256 reg) {
		__m128 hi = _mm256_extractf128_ps(reg, 1);
		__m128 lo = _mm256_extractf128_ps(reg, 0);
		lo = _mm_add_ps(hi, lo);
		hi = _mm_movehl_ps(hi, lo);
		lo = _mm_add_ps(hi, lo);
		hi = _mm_shuffle_ps(lo, lo, 1);
		lo = _mm_add_ss(hi, lo);
		return _mm_cvtss_f32(lo);
	}
}