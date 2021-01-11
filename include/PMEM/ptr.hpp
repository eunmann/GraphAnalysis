#pragma once

#include <string>

typedef unsigned long size_t;

namespace PMEM {

	enum FILE {
		CREATE,
		TEMP
	};

	/**
	 * Manages a pool of memory allocated from persistent memory
	 */
	class ptr {

	public:
		/**
		 * Allocates alloc_size in persistent memory with a temporary file-backed memory.
		 *
		 * @param alloc_size The amount of memory to try to allocate in bytes
		 */
		ptr(std::string path, FILE flag, size_t alloc_size);

		template<class T>
		T as() {
			return static_cast<T>(this->p);
		}

		void persist();

		bool is_pmem();

		size_t mapped_len();

		void resize(const size_t alloc_size);

		std::string path();

		/**
		 * Frees the persistent memory allocated
		 */
		void free();

	private:
		void* p;
		size_t m_mapped_len;
		int m_is_pmem;
		std::string m_path;
		int flags;
	};
}