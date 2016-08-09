#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_fchown32

# define SYSCALL_NR __NR_fchown32
# define SYSCALL_NAME "fchown32"
# define ACCESS_BY_DESCRIPTOR
# include "xchownx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_fchown32")

#endif
