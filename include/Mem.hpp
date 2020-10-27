#include <memkind.h>
namespace Mem {

	typedef struct MemBlock {
		struct memkind* pmem_kind;
	} MemBlock;

	void print_memkind_error(int err, const char* statement, const char* filename, int line);

	void* malloc(size_t size);

	MemBlock create_mem(size_t size);

	void delete_mem(MemBlock& memBlock);

	template<class T>
	T mem_malloc(MemBlock& memBlock, size_t size) {
		return static_cast<T>(memkind_malloc(memBlock.pmem_kind, size));
	}

	void mem_free(MemBlock& memBlock, void* ptr);
}

#define MEMKIND_CALL(statement) { \
	Mem::print_memkind_error(statement, #statement, __FILE__, __LINE__); \
}
