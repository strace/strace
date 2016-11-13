#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_syslog

# include <stdio.h>
# include <unistd.h>

# define SYSLOG_ACTION_READ 2

int
main(void)
{
	const long addr = (long) 0xfacefeeddeadbeefULL;
	int rc = syscall(__NR_syslog, SYSLOG_ACTION_READ, addr, -1);
	printf("syslog(SYSLOG_ACTION_READ, %#lx, -1) = %d %s (%m)\n",
	       addr, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_syslog")

#endif
