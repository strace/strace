/*
 * Based on test by Dr. David Alan Gilbert <dave@treblig.org>
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/syscall.h>

#undef NR_select

#if defined __NR__newselect
# define NR_select __NR__newselect
#elif defined __NR_select
# define NR_select __NR_select
#endif

#ifdef NR_select

static fd_set set[0x1000000 / sizeof(fd_set)];

int main(void)
{
	int fds[2];
	struct {
		struct timeval tv;
		int pad[2];
	} tm = {
		.tv = { .tv_usec = 123 },
		.pad = { 0xdeadbeef, 0xbadc0ded }
	};

	if (pipe(fds))
		return 77;

	/*
	 * Start with a nice simple select.
	 */
	FD_ZERO(set);
	FD_SET(fds[0], set);
	FD_SET(fds[1], set);
	if (syscall(NR_select, fds[1] + 1, set, set, set, NULL) != 1)
		return 77;
	printf("select(%d, [%d %d], [%d %d], [%d %d], NULL) = 1 ()\n",
	       fds[1] + 1, fds[0], fds[1],
	       fds[0], fds[1], fds[0], fds[1]);

	/*
	 * Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO(set);
	FD_SET(fds[1],set);
	if (syscall(NR_select, -1, NULL, set, NULL, NULL) != -1)
		return 77;
	printf("select(-1, NULL, %p, NULL, NULL) "
	       "= -1 EINVAL (Invalid argument)\n", set);

	/*
	 * Another variant, with nfds exceeding FD_SETSIZE limit.
	 */
	FD_ZERO(set);
	FD_SET(fds[0],set);
	if (syscall(NR_select, FD_SETSIZE + 1, set, set + 1, NULL, &tm.tv))
		return 77;
	printf("select(%d, [%d], [], NULL, {0, 123}) = 0 (Timeout)\n",
	       FD_SETSIZE + 1, fds[0]);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
