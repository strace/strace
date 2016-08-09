#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getresuid32

# define SYSCALL_NR	__NR_getresuid32
# define SYSCALL_NAME	"getresuid32"
# define UGID_TYPE	int
# include "getresugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getresuid32")

#endif
