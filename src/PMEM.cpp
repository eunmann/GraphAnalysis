#include "PMEM.hpp"
#include <libpmem.h>
#include <stdio.h>

namespace Mem {

	PMEM::PMEM(size_t alloc_size) : pmem_ptr(nullptr) {

		/* TODO(EMU): Directory is hardcoded, this should be passed in */
		this->pmem_ptr = pmem_map_file("./tmp/", alloc_size, PMEM_FILE_CREATE | PMEM_FILE_TMPFILE, 666, &this->mapped_len, &this->is_pmem);

		if (this->pmem_ptr == nullptr) {
			printf("Somethign went wrogn with pool allocation.\n");
		}
	}

	PMEM::~PMEM() {
		this->free();
	}

	void PMEM::persist() {
		if (this->is_pmem) {
			pmem_persist(this->pmem_ptr, this->mapped_len);
		}
		else {
			pmem_msync(this->pmem_ptr, this->mapped_len);
		}
	}

	int PMEM::is_persistent() {
		return this->is_pmem;
	}

	size_t PMEM::mapped_length() {
		return this->mapped_len;
	}

	void PMEM::free() {
		if (this->pmem_ptr != nullptr) {
			pmem_unmap(this->pmem_ptr, this->mapped_len);
			this->pmem_ptr = nullptr;
		}
	}
}