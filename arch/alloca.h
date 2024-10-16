#ifdef _WIN32
// Windows used, I need to add a compatibility alias.
#include <malloc.h>
#define alloca _alloca
#elif defined __linux__
// On glibc systems, alloca.h has the function.
// I assume musl is the same.
#include <alloca.h>
#else
// I assume that stdlib.h includes the alloca header.
// This holds true on at least FreeBSD.
#include <stdlib.h>
#endif
