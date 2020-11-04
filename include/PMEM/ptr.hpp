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

		/**
		 * Destructor which will free the persistent memory upon destruction
		 */
		~ptr();

		template<class T>
		T as() {
			return static_cast<T>(this->p);
		}

		void persist();

		int is_persistent();

		size_t mapped_length();

		/**
		 * Frees the persistent memory allocated by this MemPool
		 */
		void free();

	private:
		size_t mapped_len;
		int is_pmem;
		void* p;
	};
}