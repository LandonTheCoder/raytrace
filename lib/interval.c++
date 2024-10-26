#include <rt/interval.h>

using rt::interval;

// Defined in a separate source file to avoid linker clashes
const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);
