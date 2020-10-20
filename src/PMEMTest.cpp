#include "PMEMTest.hpp"

#include <libpmemobj.h>

#include <iostream>
namespace PMEMTest {
void simpleStructWrite() {
    /* Create the memory interface (just like creating a file) */
    PMEMobjpool* pop = pmemobj_create(L"./test_memory_simple_struct", L"intro_0", PMEMOBJ_MIN_POOL, 0666);
    if (pop == NULL) {
        perror("pmemobj_create");
        return;
    }

    /* Create the root structure */
    PMEMoid root = pmemobj_root(pop, sizeof(struct my_root));
    struct my_root* rootp = (struct my_root*)pmemobj_direct(root);

    /* Read from the memory (should have been zero'd out) */
    char buf[MAX_BUF_LEN];
    scanf("%9s", buf);

    /* Write to the memrory (just like writing to a file) */

    rootp->len = strlen(buf);
    pmemobj_persist(pop, &rootp->len, sizeof(rootp->len));
    pmemobj_memcpy_persist(pop, rootp->buf, buf, rootp->len);

    pmemobj_close(pop);
}

void simpleStructRead() {
    PMEMobjpool* pop = pmemobj_open(L"./test_memory_simple_struct", L"intro_0");
    if (pop == NULL) {
        perror("pmemobj_open");
        return;
    }

    PMEMoid root = pmemobj_root(pop, sizeof(struct my_root));
    struct my_root* rootp = (struct my_root*)pmemobj_direct(root);

    if (rootp->len == strlen(rootp->buf))
        printf("%s\n", rootp->buf);

    pmemobj_close(pop);
}
}  // namespace PMEMTest