#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_setdomainname

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_setdomainname, 0, 63);
	printf("setdomainname(NULL, 63) = %ld %s (%m)\n",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setdomainname")

#endif
