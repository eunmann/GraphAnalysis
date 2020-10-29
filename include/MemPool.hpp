#include <memkind.h>

namespace Mem {

	/**
	 * Manages a pool of memory allocated from persistent memory
	 */
	class MemPool {

	public:
		/**
		 * Allocates alloc_size in persistent memory with a temporary file-backed memory.
		 * The actual amount of memory allocated is dependent on the file system and is
		 * determined by max(MEMKIND_PMEM_MIN_SIZE, alloc_size).
		 *
		 * @param alloc_size The amount of memory to try to allocate in bytes (must be a multiple of 8)
		 */
		MemPool(size_t alloc_size);

		/**
		 * Destructor which will free the persistent memory upon destruction
		 */
		~MemPool();

		/**
		 * @param num_ele The number of elements to allocate
		 * @return A pointer within the allocated memory pool
		 */
		template<class T>
		T malloc(size_t num_ele) {
			return static_cast<T>(memkind_malloc(this->pmem_kind, sizeof(T) * num_ele));
		}

		/**
		 * Frees a pointer allocated from this MemPool
		 *
		 * @param ptr A pointer to memory allocated from this MemPool
		 */
		void free_ptr(void* ptr);

		/**
		 * Frees the persistent memory allocated by this MemPool
		 */
		void free_pool();

	private:
		struct memkind* pmem_kind;
	};
}