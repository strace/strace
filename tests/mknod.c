#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_mknod

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
	long rc = syscall(__NR_mknod, TMP_FILE, 0, 0xdeadbeef);
	printf("mknod(\"%s\", 0) = %ld %s (%m)\n",
	       TMP_FILE, rc, errno2name());

	rc = syscall(__NR_mknod, TMP_FILE, -1L, 0xdeadbeef);
	printf("mknod(\"%s\", %#o) = %ld %s (%m)\n",
	       TMP_FILE, -1, rc, errno2name());

	rc = syscall(__NR_mknod, TMP_FILE, S_IFREG|0600, 0);
	printf("mknod(\"%s\", S_IFREG|0600) = %ld %s (%m)\n",
	       TMP_FILE, rc, errno2name());

	unsigned long dev =
		(unsigned long) 0xdeadbeef00000000 | makedev(1, 7);

	rc = syscall(__NR_mknod, TMP_FILE, S_IFCHR | 0400, dev);
	printf("mknod(\"%s\", S_IFCHR|0400, makedev(1, 7)) = %ld %s (%m)\n",
	       TMP_FILE, rc, errno2name());

	const unsigned long mode =
		((unsigned long) 0xfacefeedffffffff & ~S_IFMT) | S_IFBLK;
	dev = (unsigned long) 0xdeadbeefbadc0ded;

	rc = syscall(__NR_mknod, TMP_FILE, mode, dev);
	printf("mknod(\"%s\", S_IFBLK|S_ISUID|S_ISGID|S_ISVTX|%#o"
	       ", makedev(%u, %u)) = %ld %s (%m)\n",
	       TMP_FILE, (unsigned) mode & ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX),
	       major((unsigned) dev), minor((unsigned) dev),
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mknod")

#endif
