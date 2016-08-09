#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_brk

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_brk, NULL);
	printf("brk\\(NULL\\) = %#lx\n", rc);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_brk")

#endif
