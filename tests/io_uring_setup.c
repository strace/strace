/*
 * Check decoding of io_uring_setup syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "kernel_time_types.h"
#define UAPI_LINUX_IO_URING_H_SKIP_LINUX_TIME_TYPES_H
#include <linux/io_uring.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "print_fields.h"
#include "xlat.h"

#include "xlat/uring_setup_features.h"

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
	static const char path_full[] = "/dev/full";

	long rc;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct io_uring_params, params);
	const void *efault = (const void *) params + 1;

	skip_if_unavailable("/proc/self/fd/");

	int fd_full = open(path_full, O_RDONLY);
	if (fd_full < 0)
		perror_msg_and_fail("open: %s", path_full);

	sys_io_uring_setup(-1U, NULL);
	printf("io_uring_setup(%u, NULL) = %s\n", -1U, errstr);

	sys_io_uring_setup(0, efault);
	printf("io_uring_setup(%u, %p) = %s\n", 0, efault, errstr);

	fill_memory(params, sizeof(*params));
	params->flags = -1;
	sys_io_uring_setup(1, params);
	printf("io_uring_setup(%u, {flags=IORING_SETUP_IOPOLL"
	       "|IORING_SETUP_SQPOLL|IORING_SETUP_SQ_AFF|IORING_SETUP_CQSIZE"
	       "|IORING_SETUP_CLAMP|IORING_SETUP_ATTACH_WQ"
	       "|IORING_SETUP_R_DISABLED|IORING_SETUP_SUBMIT_ALL"
	       "|IORING_SETUP_COOP_TASKRUN|IORING_SETUP_TASKRUN_FLAG"
	       "|IORING_SETUP_SQE128|IORING_SETUP_CQE32"
	       "|IORING_SETUP_SINGLE_ISSUER|IORING_SETUP_DEFER_TASKRUN"
	       "|IORING_SETUP_NO_MMAP|IORING_SETUP_REGISTERED_FD_ONLY"
	       "|IORING_SETUP_NO_SQARRAY"
	       "|%#x"
	       ", sq_thread_cpu=%#x, sq_thread_idle=%u, wq_fd=%d, resv=[",
	       1, -1U - 0x1ffff, params->sq_thread_cpu, params->sq_thread_idle,
	       params->wq_fd);
	for (unsigned int i = 0; i < ARRAY_SIZE(params->resv); ++i)
		printf("%s%#x", i != 0 ? ", " : "", params->resv[i]);
	printf("]}) = %s\n", errstr);

	for (size_t i = 0; i < 2; i++) {
		memset(params, 0, sizeof(*params));

		params->wq_fd = i == 1 ? fd_full : -1;
		params->flags = i == 1 ? 32 : 0;

		rc = sys_io_uring_setup(2, params);
		printf("io_uring_setup(%u, {flags=%s, sq_thread_cpu=0"
		       ", sq_thread_idle=0",
		       2, i == 1 ? "IORING_SETUP_ATTACH_WQ" : "0");
		if (i == 1)
			printf(", wq_fd=%d<%s>", fd_full, path_full);
		if (rc < 0) {
			printf("}) = %s\n", errstr);
		} else {
			printf(", sq_entries=%u, cq_entries=%u, features=",
			       params->sq_entries,
			       params->cq_entries);
			printflags(uring_setup_features, params->features,
				   "IORING_FEAT_???");
			printf(", sq_off={head=%u, tail=%u, ring_mask=%u"
			       ", ring_entries=%u, flags=%u, dropped=%u"
			       ", array=%u",
			       params->sq_off.head,
			       params->sq_off.tail,
			       params->sq_off.ring_mask,
			       params->sq_off.ring_entries,
			       params->sq_off.flags,
			       params->sq_off.dropped,
			       params->sq_off.array);
			if (params->sq_off.resv1)
				printf(", resv1=%#x", params->sq_off.resv1);
			printf(", user_addr=%#llx",
			       (unsigned long long)
					params->sq_off.user_addr);

			printf("}, cq_off={head=%u, tail=%u, ring_mask=%u"
			       ", ring_entries=%u, overflow=%u, cqes=%u"
			       ", flags=%u",
			       params->cq_off.head,
			       params->cq_off.tail,
			       params->cq_off.ring_mask,
			       params->cq_off.ring_entries,
			       params->cq_off.overflow,
			       params->cq_off.cqes,
			       params->cq_off.flags);
			if (params->cq_off.resv1)
				printf(", resv1=%#x", params->cq_off.resv1);
			printf(", user_addr=%#llx",
			       (unsigned long long)
					params->cq_off.user_addr);

			printf("}}) = %ld<anon_inode:[io_uring]>\n", rc);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
