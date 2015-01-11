#define ARCH_sys_mmap sys_mmap_4koff
#define ARCH_WANT_SYNC_FILE_RANGE2 1
#include "32/syscallent.h"
[244] = { 1,	0,	printargs,	"cmpxchg_badaddr"	},
[245] = { 3,	0,	printargs,	"cacheflush"		},
[246 ... 259] = { },
