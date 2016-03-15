#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_flock

# include <errno.h>
# include <sys/file.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long fd = (long int) 0xdeadbeefffffffff;
	int rc = syscall(__NR_flock, fd, LOCK_SH);
	printf("flock(%d, LOCK_SH) = %d %s (%m)\n",
	       (int) fd, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_flock")

#endif
