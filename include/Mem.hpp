namespace Mem {

	void print_memkind_error(int err, const char* statement, const char* filename, int line);

	void* malloc(size_t size);
}

#define MEMKIND_CALL(statement) { \
	Mem::print_memkind_error(statement, #statement, __FILE__, __LINE__); \
}
