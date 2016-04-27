#include "tests.h"
#include <sys/syscall.h>

#if defined __NR_mlock && defined __NR_munlock

# include <stdio.h>
# include <unistd.h>

const int size = 1024;

int
main(void)
{
	const char *addr = tail_alloc(size);
	if (syscall(__NR_mlock, addr, size) == 0) {
		printf("mlock(%p, %d) = 0\n", addr, size);
	} else {
		printf("mlock(%p, %d) = -1 %s (%m)\n",
		       addr, size, errno2name());
	}

	if (syscall(__NR_munlock, addr, size) == 0) {
		printf("munlock(%p, %d) = 0\n", addr, size);
	} else {
		printf("munlock(%p, %d) = -1 %s (%m)\n",
		       addr, size, errno2name());
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_DEFINED("__NR_mlock && __NR_munlock")

#endif
