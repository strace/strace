/*
 * Check decoding of io_uring_setup syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "scno.h"

#if defined HAVE_LINUX_IO_URING_H && defined __NR_io_uring_setup

# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <linux/io_uring.h>

# include "print_fields.h"

static const char *errstr;

static long
sys_io_uring_setup(uint32_t nentries, const void *params)
{
	kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	kernel_ulong_t arg1 = fill | nentries;
	kernel_ulong_t arg2 = (unsigned long) params;

	long rc = syscall(__NR_io_uring_setup, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	long rc;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct io_uring_params, params);
	const void *efault = (const void *) params + 1;

	skip_if_unavailable("/proc/self/fd/");

	sys_io_uring_setup(-1U, NULL);
	printf("io_uring_setup(%u, NULL) = %s\n", -1U, errstr);

	sys_io_uring_setup(0, efault);
	printf("io_uring_setup(%u, %p) = %s\n", 0, efault, errstr);

	fill_memory(params, sizeof(*params));
	params->flags = -1;
	sys_io_uring_setup(1, params);
	printf("io_uring_setup(%u, {flags=IORING_SETUP_IOPOLL"
	       "|IORING_SETUP_SQPOLL|IORING_SETUP_SQ_AFF|%#x"
	       ", sq_thread_cpu=%#x, sq_thread_idle=%u, resv={",
	       1, -1U - 7, params->sq_thread_cpu, params->sq_thread_idle);
	for (unsigned int i = 0; i < ARRAY_SIZE(params->resv); ++i)
		printf("%s%#x", i ? ", " : "", params->resv[i]);
	printf("}}) = %s\n", errstr);

	memset(params, 0, sizeof(*params));
	rc = sys_io_uring_setup(2, params);
	printf("io_uring_setup(%u, {flags=0, sq_thread_cpu=0"
	       ", sq_thread_idle=0", 2);
	if (rc < 0)
		printf("}) = %s\n", errstr);
	else
		printf(", sq_entries=%u, cq_entries=%u"
		       ", sq_off={head=%u, tail=%u, ring_mask=%u"
		       ", ring_entries=%u, flags=%u, dropped=%u, array=%u}"
		       ", cq_off={head=%u, tail=%u, ring_mask=%u"
		       ", ring_entries=%u, overflow=%u, cqes=%u}"
		       "}) = %ld<anon_inode:[io_uring]>\n",
		       params->sq_entries,
		       params->cq_entries,
		       params->sq_off.head,
		       params->sq_off.tail,
		       params->sq_off.ring_mask,
		       params->sq_off.ring_entries,
		       params->sq_off.flags,
		       params->sq_off.dropped,
		       params->sq_off.array,
		       params->cq_off.head,
		       params->cq_off.tail,
		       params->cq_off.ring_mask,
		       params->cq_off.ring_entries,
		       params->cq_off.overflow,
		       params->cq_off.cqes,
		       rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_IO_URING_H && __NR_io_uring_setup")

#endif
