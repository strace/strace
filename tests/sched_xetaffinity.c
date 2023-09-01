/*
 * Check decoding of sched_getaffinity and sched_setaffinity syscalls.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"
#include <sched.h>

#if defined CPU_ISSET_S && defined CPU_ZERO_S && defined CPU_SET_S

# include <assert.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>

static const char *errstr;

static int
getaffinity(unsigned long pid, unsigned long size, void *set)
{
	int rc = syscall(__NR_sched_getaffinity, pid, size, set);
	errstr = sprintrc(rc);
	return rc;
}

static int
setaffinity(unsigned long pid, unsigned long size, void *set)
{
	int rc = syscall(__NR_sched_setaffinity, pid, size, set);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	PIDNS_TEST_INIT;

	unsigned int cpuset_size = 1;
	const pid_t pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	while (cpuset_size) {
		assert(getaffinity(pid, cpuset_size, NULL) == -1);
		if (EFAULT == errno)
			break;
		if (EINVAL != errno)
			perror_msg_and_skip("sched_getaffinity");
		pidns_print_leader();
		printf("sched_getaffinity(%d%s, %u, NULL) = %s\n",
		       pid, pid_str, cpuset_size, errstr);
		cpuset_size <<= 1;
	}
	assert(cpuset_size);
	pidns_print_leader();
	printf("sched_getaffinity(%d%s, %u, NULL) = %s\n",
	       pid, pid_str, cpuset_size, errstr);

	cpu_set_t *cpuset = tail_alloc(cpuset_size);
	getaffinity(pid, cpuset_size, cpuset + 1);
	pidns_print_leader();
	printf("sched_getaffinity(%d%s, %u, %p) = %s\n",
	       pid, pid_str, cpuset_size, cpuset + 1, errstr);

	int ret_size = getaffinity(pid, cpuset_size, cpuset);
	if (ret_size < 0)
		perror_msg_and_fail("sched_getaffinity(%d, %u, %p) = %s\n",
				    pid, (unsigned) cpuset_size, cpuset, errstr);
	assert(ret_size <= (int) cpuset_size);

	pidns_print_leader();
	printf("sched_getaffinity(%d%s, %u, [", pid, pid_str, cpuset_size);
	const char *sep;
	unsigned int i, cpu;
	unsigned int first_cpu = -1U;
	unsigned int first_crop_cpu = -1U;
	for (i = 0, cpu = 0, sep = ""; i < (unsigned) ret_size * 8; ++i) {
		if (CPU_ISSET_S(i, (unsigned) ret_size, cpuset)) {
			printf("%s%u", sep, i);
			sep = " ";
			cpu = i;
			if (first_cpu == -1U)
				first_cpu = i;
			if (first_crop_cpu == -1U && i >= 8)
				first_crop_cpu = i;
		}
	}
	printf("]) = %s\n", errstr);

	long rc = setaffinity(pid, 0, ((char *) cpuset) + cpuset_size);
	pidns_print_leader();
	printf("sched_setaffinity(%d%s, 0, []) = %s\n",
	       pid, pid_str, sprintrc(rc));

	rc = setaffinity(pid, 1, ((char *) cpuset) + cpuset_size);
	pidns_print_leader();
	printf("sched_setaffinity(%d%s, 1, %p) = %s\n",
	       pid, pid_str, ((char *) cpuset) + cpuset_size, sprintrc(rc));

	static const uint8_t first_oob = BE_LE(SIZEOF_LONG == 4 ? 39 : 7, 56);
	const unsigned int crop_size = 8;
	cpu_set_t *crop_cpuset = tail_alloc(crop_size);
	if (first_crop_cpu != -1U && first_crop_cpu < 56) {
		CPU_ZERO_S(crop_size, crop_cpuset);
		CPU_SET_S(first_crop_cpu, crop_size, crop_cpuset);
		CPU_SET_S(first_oob, crop_size, crop_cpuset);
		if (setaffinity(pid, crop_size - 1, crop_cpuset))
			perror_msg_and_skip("sched_setaffinity()");
		pidns_print_leader();
		printf("sched_setaffinity(%d%s, 7, [%u]) = 0\n",
		       pid, pid_str, first_crop_cpu);
	}

	CPU_ZERO_S(cpuset_size, cpuset);
	CPU_SET_S(cpu, cpuset_size, cpuset);
	if (setaffinity(pid, cpuset_size, cpuset))
		perror_msg_and_skip("sched_setaffinity");
	pidns_print_leader();
	printf("sched_setaffinity(%d%s, %u, [%u]) = 0\n",
	       pid, pid_str, cpuset_size, cpu);

	const unsigned int big_size = cpuset_size < 128 ? 128 : cpuset_size * 2;
	cpuset = tail_alloc(big_size);
	ret_size = getaffinity(pid, big_size, cpuset);
	if (ret_size < 0)
		perror_msg_and_fail("sched_getaffinity(%d, %u, %p) = %s\n",
				    pid, big_size, cpuset, errstr);
	assert(ret_size <= (int) big_size);
	pidns_print_leader();
	printf("sched_getaffinity(%d%s, %u, [", pid, pid_str, big_size);
	for (i = 0, sep = ""; i < (unsigned) ret_size * 8; ++i) {
		if (CPU_ISSET_S(i, (unsigned) ret_size, cpuset)) {
			printf("%s%u", sep, i);
			sep = " ";
		}
	}
	printf("]) = %s\n", errstr);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("CPU_ISSET_S && CPU_ZERO_S && CPU_SET_S")

#endif
