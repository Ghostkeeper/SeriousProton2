#include <sp2/assert.h>
#include <cstdlib>
#include <cstdio>

void __sp2assert(const char* e, const char* filename, const char* function, int line_number)
{
    fprintf(stderr, "Assertion failed: [%s] at %s:%s:%d\n", e, filename, function, line_number);
    std::exit(1);
}
