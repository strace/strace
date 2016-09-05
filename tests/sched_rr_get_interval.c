#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_sched_rr_get_interval

# include <stdio.h>
# include <sched.h>
# include <unistd.h>

int
main(void)
{
	struct timespec *const tp = tail_alloc(sizeof(struct timespec));
	long rc = syscall(__NR_sched_rr_get_interval, -1, tp);
	printf("sched_rr_get_interval(-1, %p) = %s\n", sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_rr_get_interval")

#endif
