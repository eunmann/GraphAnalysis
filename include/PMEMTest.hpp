#include <libpmemobj.h>

namespace PMEMTest {
/* Define a format for the saved memory */
const int MAX_BUF_LEN = 10;
struct my_root {
    int len; /* = strlen(buf) */
    char buf[MAX_BUF_LEN];
};

POBJ_LAYOUT_BEGIN(string_store);
POBJ_LAYOUT_ROOT(string_store, struct my_root_2);
POBJ_LAYOUT_END(string_store);

struct my_root_2 {
    char buf[MAX_BUF_LEN];
};

void simpleStructWrite();
void simpleStructRead();
void simpleStructWrite2();
void simpleStructRead2();
}  // namespace PMEMTest