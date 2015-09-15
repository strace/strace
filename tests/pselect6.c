/*
 * Based on test by Dr. David Alan Gilbert <dave@treblig.org>
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>

#ifndef NSIG
# warning NSIG is not defined, using 32
# define NSIG 32
#endif

static fd_set set[3][0x1000000 / sizeof(fd_set)];

int main(int ac, char **av)
{
	int fds[2];
	struct timespec timeout = { .tv_sec = 0, .tv_nsec = 100 };
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGCHLD);

	assert(pipe(fds) == 0);

	/*
	 * Start with a nice simple pselect.
	 */
	FD_SET(fds[0], set[0]);
	FD_SET(fds[1], set[0]);
	FD_SET(fds[0], set[1]);
	FD_SET(fds[1], set[1]);
	FD_SET(1, set[2]);
	FD_SET(2, set[2]);
	assert(pselect(fds[1] + 1, set[0], set[1], set[2], NULL, NULL) == 1);
	printf("pselect6(%d, [%d %d], [%d %d], [1 2], NULL, {NULL, %u}) "
	       "= 1 (out [%d])\n",
	       fds[1] + 1, fds[0], fds[1],
	       fds[0], fds[1],
	       NSIG / 8, fds[1]);

	/*
	 * Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO(set[0]);
	FD_SET(fds[1],set[0]);
	assert(pselect(-1, NULL, set[0], NULL, NULL, &mask) == -1);
	printf("pselect6(-1, NULL, %p, NULL, NULL, {[HUP CHLD], %u}) "
	       "= -1 EINVAL (Invalid argument)\n", set[0], NSIG / 8);

	/*
	 * Another variant, with nfds exceeding FD_SETSIZE limit.
	 */
	FD_ZERO(set[0]);
	FD_SET(fds[0],set[0]);
	FD_ZERO(set[1]);
	assert(pselect(FD_SETSIZE + 1, set[0], set[1], NULL, &timeout, &mask) == 0);
	printf("pselect6(%d, [%d], [], NULL, {0, 100}, {[HUP CHLD], %u}) "
	       "= 0 (Timeout)\n", FD_SETSIZE + 1, fds[0], NSIG / 8);

	puts("+++ exited with 0 +++");
	return 0;
}
