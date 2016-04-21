#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_mkdir

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "mkdir";

	long rc = syscall(__NR_mkdir, sample, 0600);
	printf("mkdir(\"%s\", 0600) = %ld %s (%m)\n",
	       sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mkdir")

#endif
