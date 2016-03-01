#include "tests.h"
#include <errno.h>
#include <sys/syscall.h>

#ifdef __NR_rename

# include <stdio.h>
# include <unistd.h>

# define OLD_FILE "rename_old"
# define NEW_FILE "rename_new"

int
main(void)
{
	int rc = syscall(__NR_rename, OLD_FILE, NEW_FILE);
	printf("rename(\"%s\", \"%s\") = %d %s (%m)\n",
	       OLD_FILE, NEW_FILE, rc,
	       errno == ENOSYS ? "ENOSYS" : "ENOENT");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_rename")

#endif
