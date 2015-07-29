#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/syscall.h>

#if defined __NR_sched_getattr && defined __NR_sched_setattr

int
main(void)
{
	static union {
		struct {
			uint32_t size;
			uint32_t sched_policy;
			uint64_t sched_flags;
			uint32_t sched_nice;
			uint32_t sched_priority;
			uint64_t sched_runtime;
			uint64_t sched_deadline;
			uint64_t sched_period;
		} attr;
		char buf[256];
	} sched;

	if (syscall(__NR_sched_getattr, 0, &sched, sizeof(sched), 0))
		return 77;

	printf("sched_getattr\\(0, \\{size=%u, sched_policy=SCHED_[A-Z]+, sched_flags=%s, sched_nice=%u, sched_priority=%u, sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 "\\}, 256, 0\\) += 0\n",
		sched.attr.size,
		sched.attr.sched_flags ? "SCHED_FLAG_RESET_ON_FORK" : "0",
		sched.attr.sched_nice,
		sched.attr.sched_priority,
		sched.attr.sched_runtime,
		sched.attr.sched_deadline,
		sched.attr.sched_period);

	sched.attr.sched_flags |= 1;
	if (syscall(__NR_sched_setattr, 0, &sched, 0))
		return 77;

	printf("sched_setattr\\(0, \\{size=%u, sched_policy=SCHED_[A-Z]+, sched_flags=%s, sched_nice=%u, sched_priority=%u, sched_runtime=%" PRIu64 ", sched_deadline=%" PRIu64 ", sched_period=%" PRIu64 "\\}, 0\\) += 0\n",
		sched.attr.size,
		"SCHED_FLAG_RESET_ON_FORK",
		sched.attr.sched_nice,
		sched.attr.sched_priority,
		sched.attr.sched_runtime,
		sched.attr.sched_deadline,
		sched.attr.sched_period);

	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
