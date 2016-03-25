#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_mknod

# include <errno.h>
# include <stdio.h>
# include <sys/stat.h>
# include <unistd.h>

# define TMP_FILE "mknod"

int
main(void)
{
	int rc = syscall(__NR_mknod, TMP_FILE, S_IFREG|0600, 0);
	printf("mknod(\"%s\", S_IFREG|0600) = %d %s (%m)\n",
	       TMP_FILE, rc,
	       errno == ENOSYS ? "ENOSYS" : "EEXIST");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mknod")

#endif
