#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_symlink

# include <stdio.h>
# include <unistd.h>

int
main(int ac, char **av)
{
	static const char sample[] = "symlink.sample";

	long rc = syscall(__NR_symlink, sample, av[0]);
	printf("symlink(\"%s\", \"%s\") = %s\n", sample, av[0], sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_symlink")

#endif
