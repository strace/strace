#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_chown32

# define SYSCALL_NR __NR_chown32
# define SYSCALL_NAME "chown32"
# include "xchownx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_chown32")

#endif
