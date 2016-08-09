#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_symlink

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample_1[] = "symlink_new";
	static const char sample_2[] = "symlink";

	long rc = syscall(__NR_symlink, sample_1, sample_2);
	printf("symlink(\"%s\", \"%s\") = %ld %s (%m)\n",
	       sample_1, sample_2, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_symlink")

#endif
