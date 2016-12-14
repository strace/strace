/*
 * This file is part of sched_xetaffinity strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
#include <asm/unistd.h>
#include <sched.h>

#if defined __NR_sched_getaffinity && defined __NR_sched_setaffinity \
 && defined CPU_ISSET_S && defined CPU_ZERO_S && defined CPU_SET_S

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
	unsigned int cpuset_size = 1;
	const pid_t pid = getpid();

	while (cpuset_size) {
		assert(getaffinity(pid, cpuset_size, NULL) == -1);
		if (EFAULT == errno)
			break;
		if (EINVAL != errno)
			perror_msg_and_skip("sched_getaffinity");
		printf("sched_getaffinity(%d, %u, NULL) = %s\n",
		       pid, cpuset_size, errstr);
		cpuset_size <<= 1;
	}
	assert(cpuset_size);
	printf("sched_getaffinity(%d, %u, NULL) = %s\n",
	       pid, cpuset_size, errstr);

	cpu_set_t *cpuset = tail_alloc(cpuset_size);
	getaffinity(pid, cpuset_size, cpuset + 1);
	printf("sched_getaffinity(%d, %u, %p) = %s\n",
	       pid, cpuset_size, cpuset + 1, errstr);

	int ret_size = getaffinity(pid, cpuset_size, cpuset);
	if (ret_size < 0)
		perror_msg_and_fail("sched_getaffinity(%d, %u, %p) = %s\n",
				    pid, (unsigned) cpuset_size, cpuset, errstr);
	assert(ret_size <= (int) cpuset_size);

	printf("sched_getaffinity(%d, %u, [", pid, cpuset_size);
	const char *sep;
	unsigned int i, cpu;
	for (i = 0, cpu = 0, sep = ""; i < (unsigned) ret_size * 8; ++i) {
		if (CPU_ISSET_S(i, (unsigned) ret_size, cpuset)) {
			printf("%s%u", sep, i);
			sep = ", ";
			cpu = i;
		}
	}
	printf("]) = %s\n", errstr);

	CPU_ZERO_S(cpuset_size, cpuset);
	CPU_SET_S(cpu, cpuset_size, cpuset);
	if (setaffinity(pid, cpuset_size, cpuset))
		perror_msg_and_skip("sched_setaffinity");
	printf("sched_setaffinity(%d, %u, [%u]) = 0\n",
	       pid, cpuset_size, cpu);

	const unsigned int big_size = cpuset_size < 128 ? 128 : cpuset_size * 2;
	cpuset = tail_alloc(big_size);
	ret_size = getaffinity(pid, big_size, cpuset);
	if (ret_size < 0)
		perror_msg_and_fail("sched_getaffinity(%d, %u, %p) = %s\n",
				    pid, big_size, cpuset, errstr);
	assert(ret_size <= (int) big_size);
	printf("sched_getaffinity(%d, %u, [", pid, big_size);
	for (i = 0, sep = ""; i < (unsigned) ret_size * 8; ++i) {
		if (CPU_ISSET_S(i, (unsigned) ret_size, cpuset)) {
			printf("%s%u", sep, i);
			sep = ", ";
		}
	}
	printf("]) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_getaffinity && __NR_sched_setaffinity"
		    " && CPU_ISSET_S && CPU_ZERO_S && CPU_SET_S")

#endif
