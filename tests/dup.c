#include "tests.h"
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	int rc = dup(-1);
	printf("dup(-1) = %d %s (%m)\n", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
