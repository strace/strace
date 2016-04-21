#include "tests.h"
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	pid_t pid = getpid();
	printf("getsid(%d) = %d\n", pid, getsid(pid));

	puts("+++ exited with 0 +++");
	return 0;
}
