#include <stdio.h>
#include <sys/stat.h>

int
main(void)
{
	mode_t rc = umask(044);
	printf("umask(%#o) = %#o\n", 044, rc);

	puts("+++ exited with 0 +++");
	return 0;
}
