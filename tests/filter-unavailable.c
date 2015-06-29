#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define P 16
#define T 7

static void *
thread(void *arg)
{
	assert(write(1, "", 1) == 1);
	pause();
	return arg;
}

static int
process(void)
{
	int i;
	int fds[2];
	pthread_t t;
	struct timespec ts = { .tv_nsec = 10000000 };

	(void) close(0);
	(void) close(1);
	assert(pipe(fds) == 0 && fds[0] == 0 && fds[1] == 1);

	for (i = 0; i < T; ++i)
		assert(pthread_create(&t, NULL, thread, NULL) == 0);
	for (i = 0; i < T; ++i)
		assert(read(0, fds, 1) == 1);

	(void) nanosleep(&ts, 0);
	return 0;
}

int
main(void)
{
	int i, s;
	pid_t p;

	for (i = 0; i < P; ++i) {
		assert((p = fork()) >= 0);
		if (p == 0)
			return process();
		assert(waitpid(p, &s, 0) == p && WIFEXITED(s));
		if (WEXITSTATUS(s))
			return WEXITSTATUS(s);
	}
	return 0;
}
