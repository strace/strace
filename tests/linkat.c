#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_linkat

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample_1[] = "linkat_sample_old";
	static const char sample_2[] = "linkat_sample_new";
	const long fd_old = (long) 0xdeadbeefffffffff;
	const long fd_new = (long) 0xdeadbeeffffffffe;

	int rc = syscall(__NR_linkat, fd_old, sample_1, fd_new, sample_2, 0);
	printf("linkat(%d, \"%s\", %d, \"%s\", 0) = %d %s (%m)\n",
	       (int) fd_old, sample_1, (int) fd_new, sample_2, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_linkat")

#endif
