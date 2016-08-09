#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setresuid

# define SYSCALL_NR	__NR_setresuid
# define SYSCALL_NAME	"setresuid"

# if defined __NR_setresuid32 && __NR_setresuid != __NR_setresuid32
#  define UGID_TYPE	short
#  define GETUGID	syscall(__NR_geteuid)
#  define CHECK_OVERFLOWUGID(arg)	check_overflowuid(arg)
# else
#  define UGID_TYPE	int
#  define GETUGID	geteuid()
#  define CHECK_OVERFLOWUGID(arg)
# endif

# include "setresugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setresuid")

#endif
