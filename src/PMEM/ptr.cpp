#include "PMEM/ptr.hpp"
#include <libpmem.h>
#include <stdio.h>

namespace PMEM {

	ptr::ptr(std::string path, FILE flag, size_t alloc_size) : p(nullptr) {

		int pmem_flag = PMEM_FILE_CREATE;
		if (flag == FILE::TEMP) {
			pmem_flag |= PMEM_FILE_TMPFILE;
		}

		this->p = pmem_map_file(path.c_str(), alloc_size, pmem_flag, 0666, &this->mapped_len, &this->is_pmem);

		if (this->p == nullptr) {
			printf("Unable to mmap at %s of size %lu.\n", path.c_str(), alloc_size);
		}
	}

	ptr::~ptr() {
		this->free();
	}

	void ptr::persist() {
		if (this->is_pmem) {
			pmem_persist(this->p, this->mapped_len);
		}
		else {
			pmem_msync(this->p, this->mapped_len);
		}
	}

	int ptr::is_persistent() {
		return this->is_pmem;
	}

	size_t ptr::mapped_length() {
		return this->mapped_len;
	}

	void ptr::free() {
		if (this->p != nullptr) {
			pmem_unmap(this->p, this->mapped_len);
			this->p = nullptr;
		}
	}
}