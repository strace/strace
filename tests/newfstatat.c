#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR_newfstatat
# define TEST_SYSCALL_NAME newfstatat
#endif

#include "fstatat.c"
