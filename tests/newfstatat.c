#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/syscall.h>

#undef FSTATAT_NAME
#ifdef __NR_newfstatat
# define FSTATAT_NAME "newfstatat"
#endif

#include "fstatat.c"
