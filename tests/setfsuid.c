#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setfsuid

# define SYSCALL_NR	__NR_setfsuid
# define SYSCALL_NAME	"setfsuid"

# if defined __NR_setfsuid32 && __NR_setfsuid != __NR_setfsuid32
#  define UGID_TYPE	short
#  define GETUGID	syscall(__NR_geteuid)
# else
#  define UGID_TYPE	int
#  define GETUGID	geteuid()
# endif

# include "setfsugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setfsuid")

#endif
