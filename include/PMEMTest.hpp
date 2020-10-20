namespace PMEMTest {
/* Define a format for the saved memory */
const int MAX_BUF_LEN = 10;
struct my_root {
    int len; /* = strlen(buf) */
    char buf[MAX_BUF_LEN];
};
void simpleStructWrite();
void simpleStructRead();
}  // namespace PMEMTest