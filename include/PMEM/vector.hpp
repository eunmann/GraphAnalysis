#pragma once

#include <memory>

#include "PMEM/ptr.hpp"

typedef unsigned long size_t;

namespace PMEM {
	template<class T>
	class vector {
	public:

		vector(std::string directory, const size_t capacity) :
			data(nullptr),
			pmem(directory, PMEM::FILE::TEMP, capacity * sizeof(T)),
			m_size(0),
			m_capacity(capacity) {
			this->data = this->pmem.as<T*>();
		}

		size_t size() const {
			return this->m_size;
		}

		T& operator[](const size_t index) const {
			return this->data[index];
		}

		void resize(const size_t size) {
			this->reserve(size);
			this->m_size = size;
		}
		void reserve(const size_t capacity) {
			if (this->m_capacity < capacity) {
				this->m_capacity = capacity;
				this->pmem.resize(this->m_capacity * sizeof(T));
				this->data = this->pmem.as<T*>();
			}
		}

		void push_back(const T val) {
			if (this->m_size + 1 > this->m_capacity) {
				this->reserve(this->m_capacity * 2);
			}

			this->data[this->m_size] = val;
			this->m_size++;
		}

		void free() {
			this->pmem.free();
			this->data = nullptr;
			this->m_size = 0;
			this->m_capacity = 0;
		}

		bool is_pmem() const {
			return this->pmem.is_pmem();
		}

		/* TODO(EMU): Add a trim function to make capacity equal to size */

	private:
		T* data;
		PMEM::ptr pmem;
		size_t m_size;
		size_t m_capacity;
	};
}