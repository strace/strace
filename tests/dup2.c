#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_dup2

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd_old = (long int) 0xdeadbeefffffffffULL;
	const long int fd_new = (long int) 0xdeadbeeffffffffeULL;

	long rc = syscall(__NR_dup2, fd_old, fd_new);
	printf("dup2(%d, %d) = %ld %s (%m)\n",
	       (int) fd_old, (int) fd_new, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_dup2")

#endif
