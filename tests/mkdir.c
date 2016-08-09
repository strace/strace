#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_mkdir

# define TEST_SYSCALL_NR __NR_mkdir
# define TEST_SYSCALL_STR "mkdir"
# include "umode_t.c"

#else

SKIP_MAIN_UNDEFINED("__NR_mkdir")

#endif
