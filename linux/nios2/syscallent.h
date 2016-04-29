#define sys_ARCH_mmap sys_mmap_pgoff
#include "32/syscallent.h"
/* [244 ... 259] are arch specific */
[244] = {4,    0,	SEN(cacheflush), "cacheflush"},
