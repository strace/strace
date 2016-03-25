#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_faccessat

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

# define TMP_FILE "faccessat_tmpfile"

int
main(void)
{
	const long int fd = (long int) 0xdeadbeefffffffff;
	int rc = syscall(__NR_faccessat, fd, TMP_FILE, F_OK);
	printf("faccessat(%d, \"%s\", F_OK) = %d %s (%m)\n",
	       (int) fd, TMP_FILE, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_faccessat")

#endif
