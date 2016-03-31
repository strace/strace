#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_sethostname

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const char *hostname = NULL;
	int rc = syscall(__NR_sethostname, hostname, 63);
	const char *errno_text;
	switch (errno) {
		case ENOSYS:
			errno_text = "ENOSYS";
			break;
		case EPERM:
			errno_text = "EPERM";
			break;
		default:
			errno_text = "EFAULT";
	}
	printf("sethostname(NULL, %d) = %d %s (%m)\n",
	       63, rc, errno_text);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sethostname")

#endif
