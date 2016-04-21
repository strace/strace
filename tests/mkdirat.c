#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_mkdirat

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "mkdirat.sample";
	const long fd = (long) 0xdeadbeefffffffff;

	long rc = syscall(__NR_mkdirat, fd, sample, 0600);
	printf("mkdirat(%d, \"%s\", 0600) = %ld %s (%m)\n",
	       (int) fd, sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mkdirat")

#endif
