#include "tests.h"

#include <stdio.h>
#include <sys/mman.h>

int
main(void)
{
	printf("munlockall() = %s\n", sprintrc(munlockall()));

	puts("+++ exited with 0 +++");
	return 0;
}
