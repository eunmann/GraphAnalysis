#include "Mem.hpp"

#include <stdio.h>
#include <memkind.h>

namespace Mem {

	int print_memkind_error(int err, const char* statement, const char* filename, int line) {
		if (err != 0) {
			char error_message[MEMKIND_ERROR_MESSAGE_SIZE];
			memkind_error_message(err, error_message, MEMKIND_ERROR_MESSAGE_SIZE);
			fprintf(stderr, "%s::%d::%s - %s\n", filename, line, statement, error_message);
		}
		return err;
	}

	void* malloc(size_t size) {

		memkind_t kind;

		if (persistent_memory_available()) {
			kind = MEMKIND_DAX_KMEM;
		}
		else {
			kind = MEMKIND_DEFAULT;
		}

		return memkind_malloc(kind, size);
	}

	bool persistent_memory_available() {
		return memkind_check_available(MEMKIND_DAX_KMEM) == 0;
	}
}