#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getresgid32

# define SYSCALL_NR	__NR_getresgid32
# define SYSCALL_NAME	"getresgid32"
# define UGID_TYPE	int
# include "getresugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getresgid32")

#endif
