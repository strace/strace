#include "tests.h"
#include <sys/syscall.h>

#if defined __NR_getpriority && defined __NR_setpriority

# include <errno.h>
# include <stdio.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <unistd.h>

int
main(void)
{
	const int pid = getpid();
	int rc = syscall(__NR_getpriority, PRIO_PROCESS,
	         (unsigned long) 0xffffffff00000000 | pid);
	printf("getpriority(PRIO_PROCESS, %d) = %d\n",
	       pid, rc);

	if ((syscall(__NR_setpriority, PRIO_PROCESS,
	    (unsigned long) 0xffffffff00000000 | pid,
	    (unsigned long) 0xffffffff00000000)) == 0) {
		printf("setpriority(PRIO_PROCESS, %d, 0) = 0\n", pid);
	} else {
		printf("setpriority(PRIO_PROCESS, %d, 0) = -1 %s (%m)\n",
		       pid, errno2name());
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpriority && _NR_setpriority")

#endif
