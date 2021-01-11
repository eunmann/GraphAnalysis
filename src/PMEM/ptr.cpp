#include "PMEM/ptr.hpp"
#include <libpmem.h>
#include <string.h>
#include <FormatUtils.hpp>
#include <stdexcept>

namespace PMEM {

	ptr::ptr(std::string path, FILE flag, size_t alloc_size) :
		p(nullptr),
		m_mapped_len(0),
		m_is_pmem(0),
		m_path(path),
		flags(PMEM_FILE_CREATE) {

		if (this->flags == FILE::TEMP) {
			this->flags |= PMEM_FILE_TMPFILE;
		}

		this->resize(alloc_size);
	}

	void ptr::persist() const {
		if (this->m_is_pmem) {
			pmem_persist(this->p, this->m_mapped_len);
		}
		else {
			pmem_msync(this->p, this->m_mapped_len);
		}
	}

	bool ptr::is_pmem() const {
		return this->m_is_pmem;
	}

	size_t ptr::mapped_len() const {
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
			this->p = pmem_map_file(this->m_path.c_str(), alloc_size, this->flags, 0666, &this->m_mapped_len, &this->m_is_pmem);
		}
		else {
			size_t t_mapped_len = 0;
			void* t_p = pmem_map_file(this->m_path.c_str(), alloc_size, this->flags, 0666, &t_mapped_len, &this->m_is_pmem);

			memcpy(t_p, this->p, this->m_mapped_len > t_mapped_len ? t_mapped_len : this->m_mapped_len);
			pmem_unmap(this->p, this->m_mapped_len);
			this->p = t_p;
			this->m_mapped_len = t_mapped_len;
		}

		if (this->p == nullptr) {
			throw std::runtime_error(FormatUtils::format("Unable to mmap at %s of size %lu", this->m_path.c_str(), alloc_size));
		}

		if (this->m_mapped_len != alloc_size) {
			throw std::runtime_error(FormatUtils::format("Unable to mmap requested size. alloc_size=%lu, m_mapped_len=%lu", this->m_mapped_len, alloc_size));
		}
	}

	std::string ptr::path() const {
		return this->m_path;
	}
}