#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_getrlimit

# define NR_GETRLIMIT	__NR_getrlimit
# define STR_GETRLIMIT	"getrlimit"
# include "xgetrlimit.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getrlimit")

#endif
