#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef FSTATAT_NAME
#ifdef __NR_fstatat64
# define FSTATAT_NAME "fstatat64"
#endif

#include "fstatat.c"
