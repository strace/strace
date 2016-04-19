#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_geteuid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("geteuid() = %ld\n", syscall(__NR_geteuid));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_geteuid")

#endif
