#include "MemPool.hpp"
#include <algorithm>
#include "Mem.hpp"

namespace Mem {

	MemPool::MemPool(size_t alloc_size) {
		/* TODO(EMU): Directory is hardcoded, this should be passed in */
		MEMKIND_CALL(memkind_create_pmem("./tmp/", std::max(static_cast<size_t>(MEMKIND_PMEM_MIN_SIZE), alloc_size), &this->pmem_kind));
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