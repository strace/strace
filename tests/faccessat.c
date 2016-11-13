#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_faccessat

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "faccessat.sample";
	const long int fd = (long int) 0xdeadbeefffffffffULL;

	long rc = syscall(__NR_faccessat, fd, sample, F_OK);
	printf("faccessat(%d, \"%s\", F_OK) = %ld %s (%m)\n",
	       (int) fd, sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_faccessat")

#endif
