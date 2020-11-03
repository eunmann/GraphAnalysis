#include "PMEM.hpp"
#include <libpmem.h>
#include <stdio.h>

namespace Mem {

	PMEM::PMEM(std::string path, PMEM_FILE flag, size_t alloc_size) : pmem_ptr(nullptr) {

		int pmem_flag = PMEM_FILE_CREATE;
		if (flag == PMEM_FILE::TEMP) {
			pmem_flag |= PMEM_FILE_TMPFILE;
		}

		this->pmem_ptr = pmem_map_file(path.c_str(), alloc_size, pmem_flag, 0666, &this->mapped_len, &this->is_pmem);

		if (this->pmem_ptr == nullptr) {
			printf("Unable to mmap at %s of size %lu.\n", path.c_str(), alloc_size);
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