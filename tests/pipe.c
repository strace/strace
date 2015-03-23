#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int
main(void)
{
	(void) close(0);
	(void) close(1);
	int fds[2];
	if (pipe(fds) || fds[0] != 0 || fds[1] != 1)
		return 77;

#ifdef HAVE_PIPE2
	(void) close(0);
	(void) close(1);
	if (pipe2(fds, O_NONBLOCK) || fds[0] != 0 || fds[1] != 1)
		return 77;
	return 0;
#else
	return 77;
#endif
}
