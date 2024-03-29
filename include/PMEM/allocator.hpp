#pragma once

#include <limits>
#include <unordered_map>
#include <libpmem.h>
#include <cerrno>
#include <stdio.h>
#include <cstring>

namespace PMEM {

	template<typename T>
	class allocator {
	public:

		typedef T value_type;

		allocator() = default;
		template <typename U> constexpr allocator(const allocator<U>& a) noexcept {
			a.m_mem_size_map = this->m_mem_size_map;
		}

		T* allocate(std::size_t n) {

			if (n == 0) {
				return nullptr;
			}

			if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
				throw std::bad_alloc();
			}

			std::size_t mapped_len = 0;
			/* Note that the directory of the mounted PMEM is hardcoded here */
			if (auto p = static_cast<T*>(pmem_map_file("/pmem/", n * sizeof(T), (PMEM_FILE_CREATE | PMEM_FILE_TMPFILE), 0666, &mapped_len, &this->m_is_pmem))) {
				this->m_mem_size_map.insert(std::make_pair(p, mapped_len));
				return p;
			}

			printf("ERROR: %s\n", std::strerror(errno));

			throw std::bad_alloc();
		}

		void deallocate(T* p, std::size_t n) noexcept {
			auto pair = this->m_mem_size_map.find(p);
			if (pair != this->m_mem_size_map.end()) {
				pmem_unmap(p, pair->second);
				this->m_mem_size_map.erase(pair);
			}
		}

		int is_pmem() {
			return this->m_is_pmem;
		}

		bool operator==(const PMEM::allocator<T>& a) {
			return false;
		}

		bool operator!=(const PMEM::allocator<T>& a) {
			return true;
		}

	private:
		std::unordered_map<T*, std::size_t> m_mem_size_map;
		int m_is_pmem;
	};
}