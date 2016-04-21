#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_mknod

# include <errno.h>
# include <stdio.h>
# include <sys/stat.h>
# include <unistd.h>

# ifdef MAJOR_IN_SYSMACROS
#  include <sys/sysmacros.h>
# endif
# ifdef MAJOR_IN_MKDEV
#  include <sys/mkdev.h>
# endif

# define TMP_FILE "mknod"

int
main(void)
{
	int rc = syscall(__NR_mknod, TMP_FILE, S_IFREG|0600, 0);
	printf("mknod(\"%s\", S_IFREG|0600) = %d %s (%m)\n",
	       TMP_FILE, rc,
	       errno2name());

	const unsigned long dev =
		(unsigned long) 0xdeadbeef00000000 | makedev(1, 7);
	rc = syscall(__NR_mknod, TMP_FILE, S_IFCHR | 0400, dev);
	printf("mknod(\"%s\", S_IFCHR|0400, makedev(1, 7)) = %d %s (%m)\n",
	       TMP_FILE, rc,
	       errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mknod")

#endif
