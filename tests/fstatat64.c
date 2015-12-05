#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR_fstatat64
# define TEST_SYSCALL_NAME fstatat64
#endif

#include "fstatat.c"
