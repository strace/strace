#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_sethostname

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_sethostname, 0, 63);
	printf("sethostname(NULL, 63) = %ld %s (%m)\n",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sethostname")

#endif
