#define sys_ARCH_mmap sys_mmap_pgoff
#include "32/syscallent.h"
/* [244 ... 259] are arch specific */
[244] = { 3,	NF,	SEN(or1k_atomic),	"or1k_atomic"	},
