/*
 * Based on test by Dr. David Alan Gilbert <dave@treblig.org>
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

static fd_set set[0x1000000 / sizeof(fd_set)];

int main(int ac, char **av)
{
	int fds[2];
	struct timeval timeout = { .tv_sec = 0, .tv_usec = 100 };
	int is_select = ac < 2 || strcmp(av[1], "pselect6");

	assert(pipe(fds) == 0);

	/*
	 * Start with a nice simple select.
	 */
	FD_ZERO(set);
	FD_SET(fds[0], set);
	FD_SET(fds[1], set);
	assert(select(fds[1] + 1, set, set, set, NULL) == 1);
	if (is_select)
		printf("select(%d, [%d %d], [%d %d], [%d %d], NULL) = 1 ()\n",
		       fds[1] + 1, fds[0], fds[1],
		       fds[0], fds[1], fds[0], fds[1]);
	else
		printf("pselect6(%d, [%d %d], [%d %d], [%d %d], NULL, NULL) "
		       "= 1 ()\n",
		       fds[1] + 1, fds[0], fds[1],
		       fds[0], fds[1], fds[0], fds[1]);

	/*
	 * Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO(set);
	FD_SET(fds[1],set);
	assert(select(-1, NULL, set, NULL, NULL) == -1);
	if (is_select)
		printf("select(-1, NULL, %p, NULL, NULL) "
		       "= -1 EINVAL (Invalid argument)\n", set);
	else
		printf("pselect6(-1, NULL, %p, NULL, NULL, NULL) "
		       "= -1 EINVAL (Invalid argument)\n", set);

	/*
	 * Another variant, with nfds exceeding FD_SETSIZE limit.
	 */
	FD_ZERO(set);
	FD_SET(fds[0],set);
	assert(select(FD_SETSIZE + 1, set, set + 1, NULL, &timeout) == 0);
	if (is_select)
		printf("select(%d, [%d], [], NULL, {0, 100}) = 0 (Timeout)\n",
		       FD_SETSIZE + 1, fds[0]);
	else
		printf("pselect6(%d, [%d], [], NULL, {0, 100000}, NULL) "
		       "= 0 (Timeout)\n", FD_SETSIZE + 1, fds[0]);

	puts("+++ exited with 0 +++");
	return 0;
}
