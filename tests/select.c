/*
 * Based on test by Dr. David Alan Gilbert <dave@treblig.org>
 */
#include <assert.h>
#include <unistd.h>
#include <sys/select.h>

static fd_set set[0x1000000 / sizeof(fd_set)];

int main()
{
	int fds[2];
	struct timeval timeout = { .tv_sec = 0, .tv_usec = 100 };

	(void) close(0);
	(void) close(1);
	assert(pipe(fds) == 0);

	/*
	 * Start with a nice simple select.
	 */
	FD_ZERO(set);
	FD_SET(0, set);
	FD_SET(1, set);
	assert(select(2, set, set, set, NULL) == 1);

	/*
	 * Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO(set);
	FD_SET(1,set);
	assert(select(-1, NULL, set, NULL, NULL) == -1);

	/*
	 * Another variant, with nfds exceeding FD_SETSIZE limit.
	 */
	FD_ZERO(set);
	FD_SET(0,set);
	assert(select(FD_SETSIZE + 1, set, set + 1, NULL, &timeout) == 0);

	return 0;
}
