#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getpid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getpid() = %ld\n", syscall(__NR_getpid));
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpid")

#endif
