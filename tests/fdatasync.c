#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_fdatasync

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd = (long int) 0xdeadbeefffffffff;
	int rc = syscall(__NR_fdatasync, fd);
	printf("fdatasync(%d) = %d %s (%m)\n",
	       (int) fd, rc,
	       errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fdatasync")

#endif
