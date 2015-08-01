#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <poll.h>
#include <signal.h>
#include <unistd.h>

static int
test1(void)
{
	sigset_t mask;
	const struct timespec timeout = { .tv_sec = 42, .tv_nsec = 999999999 };
	struct pollfd fds[] = {
		{ .fd = 0, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 1, .events = POLLOUT | POLLWRNORM | POLLWRBAND },
		{ .fd = 3, .events = POLLIN | POLLPRI },
		{ .fd = 4, .events = POLLOUT }
	};

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);

	return ppoll(fds, sizeof(fds) / sizeof(*fds), &timeout, &mask) == 2 ? 0 : 77;
}

static int
test2(void)
{
	sigset_t mask;
	const struct timespec timeout = { .tv_sec = 0, .tv_nsec = 999 };
	struct pollfd fds[] = {
		{ .fd = 1, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 0, .events = POLLOUT | POLLWRNORM | POLLWRBAND }
	};

	sigfillset(&mask);
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);

	return ppoll(fds, sizeof(fds) / sizeof(*fds), &timeout, &mask) == 0 ? 0 : 77;
}

int
main(void)
{
	int rc;
	int fds[2];

	(void) close(0);
	(void) close(1);
	(void) close(3);
	(void) close(4);
	if (pipe(fds) || pipe(fds))
		return 77;


	if ((rc = test1()))
		return rc;

	if ((rc = test2()))
		return rc;

	return ppoll(NULL, 42, NULL, NULL) < 0 ? 0 : 77;
}
