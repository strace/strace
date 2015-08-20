#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int
main (void)
{
	if (syscall(__NR_times, 0x42) != -1)
		return 77;
	puts("times(0x42) = -1 EFAULT (Bad address)");
	puts("+++ exited with 0 +++");

	return 0;
}
