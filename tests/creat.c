#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_creat

# define TEST_SYSCALL_NR __NR_creat
# define TEST_SYSCALL_STR "creat"
# include "umode_t.c"

#else

SKIP_MAIN_UNDEFINED("__NR_creat")

#endif
