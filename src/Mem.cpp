#include "Mem.hpp"

#include <stdio.h>
#include <algorithm>

namespace Mem {

	void print_memkind_error(int err, const char* statement, const char* filename, int line) {
		if (err != 0) {
			char error_message[MEMKIND_ERROR_MESSAGE_SIZE];
			memkind_error_message(err, error_message, MEMKIND_ERROR_MESSAGE_SIZE);
			fprintf(stderr, "%s::%d::%s - %s\n", filename, line, statement, error_message);
		}
	}

	void* malloc(size_t size) {
		return memkind_malloc(MEMKIND_DEFAULT, size);
	}

	MemBlock create_mem(size_t size) {
		MemBlock memBlock;
		MEMKIND_CALL(memkind_create_pmem("./tmp/", MEMKIND_PMEM_MIN_SIZE, &memBlock.pmem_kind));
		return memBlock;
	}

	void mem_free(MemBlock& memBlock, void* ptr) {
		memkind_free(memBlock.pmem_kind, ptr);
	}

	void delete_mem(MemBlock& memBlock) {
		MEMKIND_CALL(memkind_destroy_kind(memBlock.pmem_kind));
	}
}