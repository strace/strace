#include "tests.h"
#include <sys/syscall.h>

#ifdef __NR_syslog

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

# define SYSLOG_ACTION_READ 2

int
main(void)
{
	const char *errno_text;
	const void *bufp = &errno_text;
	int rc = syscall(__NR_syslog, SYSLOG_ACTION_READ, bufp, -1);
	switch (errno) {
		case ENOSYS:
			errno_text = "ENOSYS";
			break;
		case EPERM:
			errno_text = "EPERM";
			break;
		default:
			errno_text = "EINVAL";
	}
	printf("syslog(SYSLOG_ACTION_READ, %p, -1) = %d %s (%m)\n",
	       bufp, rc, errno_text);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_syslog")

#endif
