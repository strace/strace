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
		case EINVAL: return "EINVAL";
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

	rc = syscall(__NR_swapon, sample, 42);
	printf("swapon(\"%s\", %s) = %d %s (%m)\n",
	       sample, "42", rc, error_msg(errno));

	rc = syscall(__NR_swapon, sample, SWAP_FLAG_PREFER);
	printf("swapon(\"%s\", %s) = %d %s (%m)\n",
	       sample, "SWAP_FLAG_PREFER", rc, error_msg(errno));

	rc = syscall(__NR_swapon, sample, SWAP_FLAG_PREFER | 42);
	printf("swapon(\"%s\", %s) = %d %s (%m)\n",
	       sample, "SWAP_FLAG_PREFER|42", rc, error_msg(errno));

	rc = syscall(__NR_swapon, sample, -1L);
	printf("swapon(\"%s\", %s) = %d %s (%m)\n",
	       sample,
	       "SWAP_FLAG_PREFER|SWAP_FLAG_DISCARD|SWAP_FLAG_DISCARD_ONCE"
	       "|SWAP_FLAG_DISCARD_PAGES|0xfff80000|32767",
	       rc, error_msg(errno));

	rc = syscall(__NR_swapoff, sample);
	printf("swapoff(\"%s\") = %d %s (%m)\n",
	       sample, rc, error_msg(errno));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_swapon && __NR_swapoff")

#endif
