typedef unsigned long size_t;

namespace Mem {

}

/**
 * A macro to wrap Mem::print_memkind_error
 */
#define MEMKIND_CALL(statement) { \
	Mem::print_memkind_error(statement, #statement, __FILE__, __LINE__);\
}
