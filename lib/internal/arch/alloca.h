#pragma once
#ifdef _WIN32
// Windows used, I need to add a compatibility alias.
#include <malloc.h>
// Note: I assume MSVC here, I don't know what I should do if this is called on Win+GCC or Win+Clang.
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

// For convenience and more readability. While this helps, this shows how VLAs help with clarity.
#define STACK_VLARRAY(arr_type, length) ((arr_type *)alloca(length * sizeof(arr_type)))
