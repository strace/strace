#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_iopl

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_iopl, 4);
	printf("iopl(4) = %ld %s (%m)\n", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_iopl")

#endif
