typedef unsigned long size_t;

namespace Mem {

	/**
	 * A function to wrap memkind function calls with. If there was an error, it will print
	 * that error with a message.
	 *
	 * @param err The memkind error code
	 * @param statement The statement that caused the error
	 * @param filename The file where the statement is contained
	 * @param line The line number of the statement within the file
	 */
	void print_memkind_error(int err, const char* statement, const char* filename, int line);

	/**
	 * Allocates memory from persistent memory without the need of creating file-backed memory
	 *
	 * @param size The size in bytes of memory to allocate (this needs to be a multiple of 8)
	 * @return A pointer to the allocated memory or NULL if there was an error
	 */
	void* malloc(size_t size);
}

/**
 * A macro to wrap Mem::print_memkind_error
 */
#define MEMKIND_CALL(statement) { \
	Mem::print_memkind_error(statement, #statement, __FILE__, __LINE__); \
}
