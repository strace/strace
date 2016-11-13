#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_mkdirat

# define TEST_SYSCALL_NR		__NR_mkdirat
# define TEST_SYSCALL_STR		"mkdirat"
# define TEST_SYSCALL_PREFIX_ARGS	(long int) 0xdeadbeefffffffffULL,
# define TEST_SYSCALL_PREFIX_STR	"-1, "
# include "umode_t.c"

#else

SKIP_MAIN_UNDEFINED("__NR_mkdirat")

#endif
