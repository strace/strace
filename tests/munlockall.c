#include "tests.h"

#include <stdio.h>
#include <sys/mman.h>

int
main(void)
{
	printf("munlockall() = %d\n", munlockall());

	puts("+++ exited with 0 +++");
	return 0;
}
