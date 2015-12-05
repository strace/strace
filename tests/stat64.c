#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR_stat64
# define TEST_SYSCALL_NAME stat64
# define STRUCT_STAT struct stat64
# define SAMPLE_SIZE ((libc_off_t) 43147718418)
#endif

#include "lstatx.c"
