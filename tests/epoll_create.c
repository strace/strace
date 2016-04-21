#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_epoll_create

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	int rc = syscall(__NR_epoll_create, -1);
	printf("epoll_create(-1) = %d %s (%m)\n",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_creat")

#endif
