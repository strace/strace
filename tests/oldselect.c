#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include <sys/select.h>
#include <sys/syscall.h>

#if defined __NR_select && defined __NR__newselect \
 && __NR_select != __NR__newselect \
 && !defined SPARC

int
main(void)
{
	int fds[2];
	fd_set r = {}, w = {};
	struct timeval timeout = { .tv_sec = 0, .tv_usec = 42 };
	long args[] = {
		2, (long) &r, (long) &w, 0, (long) &timeout,
		0xdeadbeef, 0xbadc0ded, 0xdeadbeef, 0xbadc0ded, 0xdeadbeef
	};

	(void) close(0);
	(void) close(1);
	if (pipe(fds))
		return 77;

	FD_SET(0, &w);
	FD_SET(1, &r);
	if (syscall(__NR_select, args))
		return 77;

	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
