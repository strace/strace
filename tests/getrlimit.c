#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getrlimit

# define NR_GETRLIMIT	__NR_getrlimit
# define STR_GETRLIMIT	"getrlimit"

# ifdef __NR_ugetrlimit
#  define INFINITY	0x7fffffff
#  define INFINITY_STR	"old getrlimit() infinity"
# else
#  define INFINITY	RLIM_INFINITY
#  define INFINITY_STR	"RLIM_INFINITY"
# endif

# include "xgetrlimit.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getrlimit")

#endif
