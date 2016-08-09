#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setresgid32

# define SYSCALL_NR	__NR_setresgid32
# define SYSCALL_NAME	"setresgid32"
# define UGID_TYPE	int
# define GETUGID	getegid()
# define CHECK_OVERFLOWUGID(arg)
# include "setresugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setresgid32")

#endif
