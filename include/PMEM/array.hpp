#pragma once

#include "PMEM/ptr.hpp"

typedef unsigned long size_t;

namespace PMEM {
	template<class T>
	class array {
	public:
		array(const size_t size) :
			data(nullptr),
			m_size(size),
			pmem(nullptr) {
			this->pmem = PMEM::ptr("./tmp/", PMEM::FILE::TEMP, size);
			this->data = this->pmem.as<T>();
		}

		size_t size() {
			return this->m_size;
		}

		T& operator[](const size_t index) {
			return this->data[index];
		}
	private:
		T* data;
		size_t m_size;
		PMEM::ptr pmem;
	};
}