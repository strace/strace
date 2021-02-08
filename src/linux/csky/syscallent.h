#include "../32/syscallent.h"
/* [244 ... 259] are arch specific */
[244] = {1,    0,	SEN(set_thread_area), "set_thread_area"},
[245] = {3,    0,	SEN(cacheflush), "cacheflush"},
