#include "tests.h"
#include <errno.h>
#include <sys/syscall.h>

#ifdef __NR_renameat

# include <stdio.h>
# include <unistd.h>

# define OLD_FILE "renameat_old"
# define NEW_FILE "renameat_new"

int
main(void)
{
	const long int fd_old = (long int) 0xdeadbeefffffffff;
	const long int fd_new = (long int) 0xdeadbeeffffffffe;
	int rc = syscall(__NR_renameat, fd_old, OLD_FILE, fd_new, NEW_FILE);
	printf("renameat(%d, \"%s\", %d, \"%s\") = %d %s (%m)\n",
	       (int) fd_old, OLD_FILE, (int) fd_new, NEW_FILE, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_renameat")

#endif
