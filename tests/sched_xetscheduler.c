#include "tests.h"
#include <sys/syscall.h>

#if defined __NR_sched_getscheduler && defined __NR_sched_setscheduler

# include <sched.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	struct sched_param *const param = tail_alloc(sizeof(struct sched_param));
	long rc = syscall(__NR_sched_getscheduler, 0);
	const char *scheduler;
	switch (rc) {
		case SCHED_FIFO:
			scheduler = "SCHED_FIFO";
			break;
		case SCHED_RR:
			scheduler = "SCHED_RR";
			break;
# ifdef SCHED_BATCH
		case SCHED_BATCH:
			scheduler = "SCHED_BATCH";
			break;
# endif
# ifdef SCHED_IDLE
		case SCHED_IDLE:
			scheduler = "SCHED_IDLE";
			break;
# endif
# ifdef SCHED_ISO
		case SCHED_ISO:
			scheduler = "SCHED_ISO";
			break;
# endif
# ifdef SCHED_DEADLINE
		case SCHED_DEADLINE:
			scheduler = "SCHED_DEADLINE";
			break;
# endif
		default:
			scheduler = "SCHED_OTHER";
	}
	printf("sched_getscheduler(0) = %ld (%s)\n",
	       rc, scheduler);

	param->sched_priority = -1;
	rc = syscall(__NR_sched_setscheduler, 0, SCHED_FIFO, param);
	printf("sched_setscheduler(0, SCHED_FIFO, [%d]) = %ld %s (%m)\n",
	       param->sched_priority, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_getscheduler && __NR_sched_setscheduler")

#endif
