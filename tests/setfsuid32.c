#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setfsuid32

# define SYSCALL_NR	__NR_setfsuid32
# define SYSCALL_NAME	"setfsuid32"
# define UGID_TYPE	int
# define GETUGID	geteuid()
# include "setfsugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setfsuid32")

#endif
