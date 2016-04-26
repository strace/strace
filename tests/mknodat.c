#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_mknodat

# include <stdio.h>
# include <sys/stat.h>
# include <unistd.h>

# ifdef MAJOR_IN_SYSMACROS
#  include <sys/sysmacros.h>
# endif
# ifdef MAJOR_IN_MKDEV
#  include <sys/mkdev.h>
# endif

int
main(void)
{
	static const char sample[] = "mknokat_sample";
	const long int fd = (long int) 0xdeadbeefffffffff;
	long rc = syscall(__NR_mknodat, fd, sample, S_IFREG|0600, 0);
	printf("mknodat(%d, \"%s\", S_IFREG|0600) = %ld %s (%m)\n",
	       (int) fd, sample, rc, errno2name());

	const unsigned long dev =
		(unsigned long) 0xdeadbeef00000000 | makedev(1, 7);

	rc = syscall(__NR_mknodat, fd, sample, S_IFCHR | 0400, dev);
	printf("mknodat(%d, \"%s\", S_IFCHR|0400, makedev(1, 7)) = %ld %s (%m)\n",
	       (int) fd, sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mknodat")

#endif
