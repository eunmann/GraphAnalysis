#include <memkind.h>

namespace Mem {
	class MemPool {

	public:
		MemPool(size_t alloc_size);
		~MemPool();

		template<class T>
		T malloc(size_t size) {
			return static_cast<T>(memkind_malloc(this->pmem_kind, size));
		}

		void free_ptr(void* ptr);
		void free_pool();

	private:
		struct memkind* pmem_kind;
	};
}