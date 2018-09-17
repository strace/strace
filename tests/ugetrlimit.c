#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_ugetrlimit

# define NR_GETRLIMIT	__NR_ugetrlimit
# define STR_GETRLIMIT	"ugetrlimit"
# define INFINITY	RLIM_INFINITY
# define INFINITY_STR	"RLIM_INFINITY"
# include "xgetrlimit.c"

#else

SKIP_MAIN_UNDEFINED("__NR_ugetrlimit")

#endif
