#include "MemPool.hpp"
#include <algorithm>
#include "Mem.hpp"

namespace Mem {

	MemPool::MemPool(size_t alloc_size) {
		/* TODO(EMU): For some reason, you cannot allocate all of the memory in the pool, so add 8 bytes here so the
			user can use all of the memory they asked for */
		alloc_size = MEMKIND_PMEM_MIN_SIZE <= alloc_size ? alloc_size + 8 : MEMKIND_PMEM_MIN_SIZE;
		/* TODO(EMU): Directory is hardcoded, this should be passed in */
		MEMKIND_CALL(memkind_create_pmem("./tmp/", alloc_size, &this->pmem_kind));
	}

	MemPool::~MemPool() {
		this->free_pool();
	}

	void MemPool::free_ptr(void* ptr) {
		memkind_free(this->pmem_kind, ptr);
	}

	void MemPool::free_pool() {
		if (this->pmem_kind != nullptr) {
			MEMKIND_CALL(memkind_destroy_kind(this->pmem_kind));
			this->pmem_kind = nullptr;
		}
	}
}