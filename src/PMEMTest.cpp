#include "PMEMTest.hpp"

#include <libpmemobj.h>

#include <iostream>

namespace PMEMTest {
void simpleStructWrite() {
    /* Create the memory interface (just like creating a file) */
    PMEMobjpool* pop = pmemobj_create("./test_memory_simple_struct", "intro_0", PMEMOBJ_MIN_POOL, 0666);
    if (pop == NULL) {
        perror("pmemobj_create");
        return;
    }

    /* Create the root structure */
    PMEMoid root = pmemobj_root(pop, sizeof(struct my_root));
    struct my_root* rootp = (struct my_root*)pmemobj_direct(root);

    /* Read Input from user */
    char buf[MAX_BUF_LEN];
    scanf("%9s", buf);

    /* Write to the memory (just like writing to a file) */
    rootp->len = strlen(buf);
    pmemobj_persist(pop, &rootp->len, sizeof(rootp->len));
    pmemobj_memcpy_persist(pop, rootp->buf, buf, rootp->len);

    /* Close the access to the memory (again, exactly like a file) */
    pmemobj_close(pop);
}

void simpleStructRead() {
    /* Open up the memory, using a path and an layout name */
    PMEMobjpool* pop = pmemobj_open("./test_memory_simple_struct", "intro_0");
    if (pop == NULL) {
        perror("pmemobj_open");
        return;
    }

    /* Get a Persistent Memory Pointer to the root object in the memory */
    PMEMoid root = pmemobj_root(pop, sizeof(struct my_root));
    /* Get a direct pointer to this memory (is this in RAM or Persistent Memory?)*/
    struct my_root* rootp = (struct my_root*)pmemobj_direct(root);

    /* Read straight from pointer */
    if (rootp->len == strlen(rootp->buf))
        printf("%s\n", rootp->buf);

    /* Close, just like a file */
    pmemobj_close(pop);
}

void simpleStructWrite2() {
    /* Create the memory interface (just like creating a file) */
    PMEMobjpool* pop = pmemobj_create("./test_memory_simple_struct_2", POBJ_LAYOUT_NAME(string_store), PMEMOBJ_MIN_POOL, 0666);
    if (pop == NULL) {
        perror("pmemobj_create");
        return;
    }

    /* Use macros from the library to create type safety pointer to persistent memory */
    TOID(struct my_root_2)
    root = POBJ_ROOT(pop, struct my_root_2);

    /* Read Input from user */
    char buf[MAX_BUF_LEN];
    scanf("%9s", buf);

    /*
		Use a transaction block to take a snapshot of the persistent memory.
		Then, attempt to write to it, if anything fails, the memory is 
		reset to the snapshot and all changes are reverted. TX means transaction.
		Also, these transactions can be nested within each other, also called recursively.
		However, you must pay careful attention to where you end up if an abort is called.
		It's similar to how error/exceptions are thrown.
	*/
    TX_BEGIN(pop) {
        /*
			Use macros for type safety.
			D_RW is Direct Read/Write.
		*/
        TX_MEMCPY(D_RW(root)->buf, buf, strlen(buf));
    }
    TX_ONCOMMIT {
        /*
			This code block executes when the above block finishes successfully.
			You know that it ran and every is saved.
		*/
    }
    TX_ONABORT {
        /*
			This code block executes when anything above fails.
			You don't know what failed exactly but logging should be done
			here.
		*/
    }
    TX_FINALLY {
        /*
			This always runs as the last block, so free calls should be done here.
		*/
    }
    TX_END

    /* Close the access to the memory (again, exactly like a file) */
    pmemobj_close(pop);
}

void simpleStructRead2() {
    PMEMobjpool* pop = pmemobj_open("./test_memory_simple_struct_2", POBJ_LAYOUT_NAME(string_store));
    if (pop == NULL) {
        perror("pmemobj_open");
        return;
    }

    /* Use macros from the library to create type safety pointer to persistent memory */
    TOID(struct my_root_2)
    root = POBJ_ROOT(pop, struct my_root_2);

    /*
		Use macros for type safety.
		D_RO is Direct Read Only.
	*/
    printf("%s\n", D_RO(root)->buf);

    /* Close the access to the memory (again, exactly like a file) */
    pmemobj_close(pop);
}
}  // namespace PMEMTest