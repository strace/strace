#include "tests.h"
#include <asm/unistd.h>

#if defined __NR_sched_getscheduler && defined __NR_sched_setscheduler

# include <sched.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sched_param, param);
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

	rc = syscall(__NR_sched_getscheduler, -1);
	printf("sched_getscheduler(-1) = %s\n", sprintrc(rc));

	param->sched_priority = -1;

	rc = syscall(__NR_sched_setscheduler, 0, SCHED_FIFO, NULL);
	printf("sched_setscheduler(0, SCHED_FIFO, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, 0, SCHED_FIFO, param + 1);
	printf("sched_setscheduler(0, SCHED_FIFO, %p) = %s\n", param + 1,
	       sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, 0, 0xfaceda7a, param);
	printf("sched_setscheduler(0, %#x /* SCHED_??? */, [%d]) = %s\n",
	       0xfaceda7a, param->sched_priority, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, -1, SCHED_FIFO, param);
	printf("sched_setscheduler(-1, SCHED_FIFO, [%d]) = %s\n",
	       param->sched_priority, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, 0, SCHED_FIFO, param);
	printf("sched_setscheduler(0, SCHED_FIFO, [%d]) = %s\n",
	       param->sched_priority, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_getscheduler && __NR_sched_setscheduler")

#endif
