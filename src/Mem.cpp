#include "Mem.hpp"

#include <stdio.h>
#include <memkind.h>

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
}