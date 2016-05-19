#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_vhangup

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_vhangup);
	printf("vhangup() = %ld %s (%m)\n", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_vhangup")

#endif
