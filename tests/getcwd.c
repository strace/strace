#include "tests.h"

#include <sys/syscall.h>

#ifdef __NR_getcwd

# include <stdio.h>
# include <unistd.h>
# include <sys/param.h>

int
main(void)
{
	long res;
	char cur_dir[PATH_MAX + 1];

	res = syscall(__NR_getcwd, cur_dir, sizeof(cur_dir));

	if (res <= 0)
		perror_msg_and_fail("getcwd");

	printf("getcwd(\"");
	print_quoted_string(cur_dir);
	printf("\", %zu) = %ld\n", sizeof(cur_dir), res);

	syscall(__NR_getcwd, cur_dir, 0);

	printf("getcwd(%p, 0) = -1 ERANGE (%m)\n", cur_dir);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getcwd");

#endif
