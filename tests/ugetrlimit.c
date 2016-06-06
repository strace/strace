#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_ugetrlimit

# define NR_GETRLIMIT	__NR_ugetrlimit
# define STR_GETRLIMIT	"ugetrlimit"
# include "xgetrlimit.c"

#else

SKIP_MAIN_UNDEFINED("__NR_ugetrlimit")

#endif
