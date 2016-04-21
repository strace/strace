#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_creat

# include <stdio.h>
# include <unistd.h>

# define TMP_FILE "creat"

int
main(void)
{
	long rc = syscall(__NR_creat, TMP_FILE, 0400);
	printf("creat(\"%s\", %#o) = %ld %s (%m)\n",
	       TMP_FILE, 0400, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_creat")

#endif
