#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_getgroups32

# include "getgroups.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getgroups32")

#endif
