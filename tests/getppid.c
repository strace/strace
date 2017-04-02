#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>

int
main(void)
{
	printf("getppid() = %ld\n", syscall(__NR_getppid));
	puts("+++ exited with 0 +++");
	return 0;
}
