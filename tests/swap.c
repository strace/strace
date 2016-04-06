#include "tests.h"
#include <sys/syscall.h>

#if defined __NR_swapon && defined __NR_swapoff

# include <errno.h>
# include <stdio.h>
# include <sys/swap.h>
# include <unistd.h>

static const char *
error_msg(int error_num)
{
	switch (error_num) {
		case ENOSYS: return "ENOSYS";
		case EPERM: return "EPERM";
		default: return "ENOENT";
	}
}

int
main(void)
{
	static const char sample[] = "swap.sample";

	int rc = syscall(__NR_swapon, sample, 0);
	printf("swapon(\"%s\", 0) = %d %s (%m)\n",
	       sample, rc, error_msg(errno));

	rc = syscall(__NR_swapoff, sample);
	printf("swapoff(\"%s\") = %d %s (%m)\n",
	       sample, rc, error_msg(errno));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_swapon && __NR_swapoff")

#endif
