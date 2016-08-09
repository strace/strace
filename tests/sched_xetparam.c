#include "tests.h"
#include <asm/unistd.h>

#if defined __NR_sched_getparam && defined __NR_sched_setparam

# include <sched.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	struct sched_param *const param =
		tail_alloc(sizeof(struct sched_param));

	long rc = syscall(__NR_sched_getparam, 0, param);
	printf("sched_getparam(0, [%d]) = %ld\n",
	       param->sched_priority, rc);

	param->sched_priority = -1;
	rc = syscall(__NR_sched_setparam, 0, param);
	printf("sched_setparam(0, [%d]) = %ld %s (%m)\n",
	       param->sched_priority, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_getparam && __NR_sched_setparam")

#endif
