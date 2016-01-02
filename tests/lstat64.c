#include "tests.h"
#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR_lstat64
# define TEST_SYSCALL_NAME lstat64
# define STRUCT_STAT struct stat64
# define SAMPLE_SIZE ((libc_off_t) 43147718418)
#endif

#include "lstatx.c"
