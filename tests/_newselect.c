#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef TEST_SYSCALL_NAME
#ifdef __NR__newselect
# define TEST_SYSCALL_NAME _newselect
#endif

#include "xselect.c"
