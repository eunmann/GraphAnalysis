#include "PMEM/ptr.hpp"
#include <libpmem.h>
#include <stdio.h>
#include <string.h>

namespace PMEM {

	ptr::ptr(std::string path, FILE flag, size_t alloc_size) :
		p(nullptr),
		m_mapped_len(0),
		is_pmem(0),
		m_path(path),
		flags(PMEM_FILE_CREATE) {

		if (this->flags == FILE::TEMP) {
			this->flags |= PMEM_FILE_TMPFILE;
		}

		this->resize(alloc_size);
	}

	void ptr::persist() {
		if (this->is_pmem) {
			pmem_persist(this->p, this->m_mapped_len);
		}
		else {
			pmem_msync(this->p, this->m_mapped_len);
		}
	}

	int ptr::is_persistent() {
		return this->is_pmem;
	}

	size_t ptr::mapped_len() {
		return this->m_mapped_len;
	}

	void ptr::free() {
		if (this->p != nullptr) {
			pmem_unmap(this->p, this->m_mapped_len);
			this->p = nullptr;
		}
	}

	void ptr::resize(const size_t alloc_size) {

		if (this->p == nullptr) {
			this->p = pmem_map_file(this->m_path.c_str(), alloc_size, this->flags, 0666, &this->m_mapped_len, &this->is_pmem);
		}
		else {
			size_t t_mapped_len = 0;
			void* t_p = pmem_map_file(this->m_path.c_str(), alloc_size, this->flags, 0666, &t_mapped_len, &this->is_pmem);

			memcpy(t_p, this->p, this->m_mapped_len > t_mapped_len ? t_mapped_len : this->m_mapped_len);
			pmem_unmap(this->p, this->m_mapped_len);
			this->p = t_p;
			this->m_mapped_len = t_mapped_len;
		}

		if (this->p == nullptr) {
			printf("Unable to mmap at %s of size %lu.\n", this->m_path.c_str(), alloc_size);
		}
	}

	std::string ptr::path() {
		return this->m_path;
	}
}