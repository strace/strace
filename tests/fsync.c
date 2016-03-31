#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_fsync

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd = (long int) 0xdeadbeefffffffff;
	int rc = syscall(__NR_fsync, fd);
	printf("fsync(%d) = %d %s (%m)\n",
	       (int) fd, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fsync")

#endif
