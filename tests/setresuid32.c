#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setresuid32

# define SYSCALL_NR	__NR_setresuid32
# define SYSCALL_NAME	"setresuid32"
# define UGID_TYPE	int
# define GETUGID	geteuid()
# define CHECK_OVERFLOWUGID(arg)
# include "setresugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setresuid32")

#endif
