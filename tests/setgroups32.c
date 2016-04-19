#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_setgroups32

# include "setgroups.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setgroups32")

#endif
