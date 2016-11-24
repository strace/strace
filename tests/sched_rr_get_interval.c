#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_sched_rr_get_interval

# include <stdint.h>
# include <stdio.h>
# include <sched.h>
# include <unistd.h>

int
main(void)
{
	struct timespec *const tp = tail_alloc(sizeof(struct timespec));
	long rc;

	rc = syscall(__NR_sched_rr_get_interval, 0, NULL);
	printf("sched_rr_get_interval(0, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_sched_rr_get_interval, 0, tp + 1);
	printf("sched_rr_get_interval(0, %p) = %s\n", tp + 1, sprintrc(rc));

	rc = syscall(__NR_sched_rr_get_interval, -1, tp);
	printf("sched_rr_get_interval(-1, %p) = %s\n", tp, sprintrc(rc));

	rc = syscall(__NR_sched_rr_get_interval, 0, tp);
	if (rc == 0)
		printf("sched_rr_get_interval(0, {tv_sec=%jd, tv_nsec=%jd}) = "
		       "0\n", (intmax_t)tp->tv_sec, (intmax_t)tp->tv_nsec);
	else
		printf("sched_rr_get_interval(-1, %p) = %s\n", tp,
			sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_rr_get_interval")

#endif
