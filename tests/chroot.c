#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_chroot

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "chroot.sample";
	int rc = syscall(__NR_chroot ,sample);
	const char *errno_text;
	switch (errno) {
		case ENOSYS:
			errno_text = "ENOSYS";
			break;
		case EPERM:
			errno_text = "EPERM";
			break;
		default:
			errno_text = "ENOENT";
	}
	printf("chroot(\"%s\") = %d %s (%m)\n",
	       sample, rc, errno_text);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_chroot")

#endif
