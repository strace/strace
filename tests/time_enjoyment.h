#ifndef STRACE_TESTS_TIME_ENJOYMENT_H
# define STRACE_TESTS_TIME_ENJOYMENT_H

# include <fcntl.h>
# include <sched.h>
# include <time.h>
# include <sys/types.h>
# include <sys/stat.h>

enum {
	NUM_USER_ITERS_SQRT = 2000,
	NUM_USER_ITERS = NUM_USER_ITERS_SQRT * NUM_USER_ITERS_SQRT,
	READ_BUF_SIZE = 65536,
	READ_ITER = 128,
};

static inline uint64_t
nsecs(struct timespec *ts)
{
	return (uint64_t) ts->tv_sec * 1000000000 + ts->tv_nsec;
}

static inline void
enjoy_time(uint64_t cputime_limit)
{
	struct timespec ts = { 0 };
	volatile int dummy = 0;

	/* Enjoying my user time */
	for (size_t i = 0; i < NUM_USER_ITERS_SQRT; ++i) {
		if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
			if (nsecs(&ts) >= cputime_limit)
				break;
		}

		for (size_t j = 0; j < NUM_USER_ITERS; ++j)
			++dummy;
	}

	/* Enjoying my system time */
	ssize_t ret;
	int fd;
	char buf[READ_BUF_SIZE];

	while (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
		for (size_t i = 0; i < READ_ITER; i++) {
			/* We are fine even if the calls fail. */
			fd = open("/proc/self/status", O_RDONLY);
			/*
			 * Working around "ignoring return value of 'read'
			 * declared with attribute 'warn_unused_result'".
			 */
			ret = read(fd, buf, sizeof(buf));
			close(fd);
			if (ret)
				continue;
		}

		if (nsecs(&ts) >= cputime_limit * 3)
			break;

		sched_yield();
	}
}

#endif /* !STRACE_TESTS_TIME_ENJOYMENT_H */
