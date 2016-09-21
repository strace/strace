#include "tests.h"

#include <asm/unistd.h>

#ifdef __NR_getcwd

# include <stdio.h>
# include <unistd.h>
# include <sys/param.h>

int
main(void)
{
	long res;
	char cur_dir[PATH_MAX + 1];
	static const size_t bogus_size = (size_t) 0xbadc0deddeadfaceULL;

	res = syscall(__NR_getcwd, cur_dir, sizeof(cur_dir));

	if (res <= 0)
		perror_msg_and_fail("getcwd");

	printf("getcwd(\"");
	print_quoted_string(cur_dir);
	printf("\", %zu) = %ld\n", sizeof(cur_dir), res);

	res = syscall(__NR_getcwd, cur_dir, 0);
	printf("getcwd(%p, 0) = %s\n", cur_dir, sprintrc(res));

	res = syscall(__NR_getcwd, NULL, bogus_size);
	printf("getcwd(NULL, %zu) = %s\n", bogus_size, sprintrc(res));

	res = syscall(__NR_getcwd, (void *) -1L, sizeof(cur_dir));
	printf("getcwd(%p, %zu) = %s\n",
	       (void *) -1L, sizeof(cur_dir), sprintrc(res));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getcwd");

#endif
