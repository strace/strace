#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setreuid32

# define SYSCALL_NR	__NR_setreuid32
# define SYSCALL_NAME	"setreuid32"
# define UGID_TYPE	int
# define GETUGID	geteuid()
# define CHECK_OVERFLOWUGID(arg)
# include "setreugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setreuid32")

#endif
