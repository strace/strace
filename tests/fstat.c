#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR_fstat
# define TEST_SYSCALL_NAME fstat
# define SAMPLE_SIZE ((kernel_ulong_t) 43147718418)
#endif

#include "fstatx.c"
