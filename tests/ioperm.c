#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_ioperm

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long port = (unsigned long) 0xdeafbeefffffffffULL;

	long rc = syscall(__NR_ioperm, port, 1, 0);
	printf("ioperm(%#lx, %#lx, %d) = %ld %s (%m)\n",
	       port, 1UL, 0, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ioperm")

#endif
