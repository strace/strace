#include "tests.h"
#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR_newfstatat
# define TEST_SYSCALL_NAME newfstatat
#endif

#include "fstatat.c"
