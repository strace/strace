#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <asm/unistd.h>

int
main(void)
{
	printf("gettid() = %ld\n", syscall(__NR_gettid));
	puts("+++ exited with 0 +++");
	return 0;
}
