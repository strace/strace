#include "tests.h"
#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#if defined __NR_select && !defined __NR__newselect
# define TEST_SYSCALL_NAME select
#endif

#include "xselect.c"
