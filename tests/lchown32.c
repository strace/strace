#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_lchown32

# define SYSCALL_NR __NR_lchown32
# define SYSCALL_NAME "lchown32"
# include "xchownx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_lchown32")

#endif
