#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	const int fd = -1;
	int rc = dup(fd);
	printf("dup(%d) = %d %s (%m)\n",
	       fd, rc,
	       errno == ENOSYS ? "ENOSYS" : "EBADF");

	puts("+++ exited with 0 +++");
	return 0;
}
