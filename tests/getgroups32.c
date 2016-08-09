#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getgroups32

# include "getgroups.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getgroups32")

#endif
