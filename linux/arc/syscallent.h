#define sys_ARCH_mmap sys_mmap_pgoff
#include "32/syscallent.h"
[244] = { 3,	0,	SEN(printargs),	"arc_cacheflush"},
[245] = { 1,	0,	SEN(printargs),	"arc_settls"	},
[246] = { 0,	0,	SEN(printargs),	"arc_gettls"	},
[247 ... 259] = { },
