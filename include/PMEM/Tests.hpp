#pragma once

#include <libpmemobj.h>

namespace PMEM::Tests {
	/* Define a format for the saved memory */
	const unsigned int MAX_BUF_LEN = 10;

	struct my_root {
		unsigned int len; /* = strlen(buf) */
		char buf[MAX_BUF_LEN];
	};

	struct my_root_2 {
		char buf[MAX_BUF_LEN];
	};

	/* 'stringstore' is a custom name for the layout */
	POBJ_LAYOUT_BEGIN(string_store);
	POBJ_LAYOUT_ROOT(string_store, struct my_root_2);
	POBJ_LAYOUT_END(string_store);

	void libpmemobj_example_write_1();
	void libpmemobj_example_read_1();
	void libpmemobj_example_write_2();
	void libpmemobj_example_read_2();

	void pmem_as_volatile_API();
	void pmemobj_vectors();
}