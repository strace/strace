#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_rmdir

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "rmdir_sample";
	long rc = syscall(__NR_rmdir, sample);
	printf("rmdir(\"%s\") = %ld %s (%m)\n",
	       sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_rmdir")

#endif
