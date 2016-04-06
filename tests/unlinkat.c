#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_unlinkat

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "unlinkat_sample";
	const long fd = (long) 0xdeadbeefffffffff;

	int rc = syscall(__NR_unlinkat, fd, sample, 0);
	printf("unlinkat(%d, \"%s\", 0) = %d %s (%m)\n",
	       (int) fd, sample, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_unlinkat")

#endif
