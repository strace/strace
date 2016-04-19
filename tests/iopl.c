#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_iopl

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_iopl, 4);
	const char *error_text;
	switch (errno) {
		case ENOSYS:
			error_text = "ENOSYS";
			break;
		case EPERM:
			error_text = "EPERM";
			break;
		default:
			error_text = "EINVAL";
	}
	printf("iopl(4) = %ld %s (%m)\n", rc, error_text);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_iopl")

#endif
