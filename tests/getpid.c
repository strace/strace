#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>

int
main(void)
{
	printf("getpid() = %ld\n", syscall(__NR_getpid));
	puts("+++ exited with 0 +++");
	return 0;
}
