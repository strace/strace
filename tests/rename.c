#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_rename

# include <stdio.h>
# include <unistd.h>

# define OLD_FILE "rename_old"
# define NEW_FILE "rename_new"

int
main(void)
{
	long rc = syscall(__NR_rename, OLD_FILE, NEW_FILE);
	printf("rename(\"%s\", \"%s\") = %ld %s (%m)\n",
	       OLD_FILE, NEW_FILE, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_rename")

#endif
