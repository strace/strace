#include "tests.h"
#include <fcntl.h>
#include <asm/unistd.h>

#if defined __NR_dup3 && defined O_CLOEXEC

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd_old = (long int) 0xdeadbeefffffffffULL;
	const long int fd_new = (long int) 0xdeadbeeffffffffeULL;

	long rc = syscall(__NR_dup3, fd_old, fd_new, O_CLOEXEC);
	printf("dup3(%d, %d, O_CLOEXEC) = %ld %s (%m)\n",
	       (int) fd_old, (int) fd_new, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_dup3 && && O_CLOEXEC")

#endif
