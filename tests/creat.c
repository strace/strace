#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_creat

# include <errno.h>
# include <stdio.h>
# include <sys/stat.h>
# include <unistd.h>

# define TMP_FILE "creat"

int
main(void)
{
	int rc = syscall(__NR_creat, TMP_FILE, S_IRUSR);
	printf("creat(\"%s\", %#o) = %d %s (%m)\n",
	       TMP_FILE, S_IRUSR, rc,
	       errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_creat")

#endif
