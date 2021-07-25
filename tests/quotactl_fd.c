/*
 * Check decoding of quotactl_fd syscall.
 *
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "xmalloc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/quota.h>
#include "xlat.h"
#include "xlat/quota_formats.h"

#ifndef DECODE_FDS
# define DECODE_FDS 0
#endif
#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

static const char *errstr;

static long
k_quotactl_fd(const unsigned int fd,
	      const unsigned int cmd,
	      const uint32_t id,
	      const void *const ptr)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd;
	const kernel_ulong_t arg2 = fill | cmd;
	const kernel_ulong_t arg3 = fill | id;
	const kernel_ulong_t arg4 = (uintptr_t) ptr;
	const long rc = syscall(__NR_quotactl_fd,
				arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	static const char fd_path[] = "/dev/full";
	int fd = open(fd_path, O_WRONLY);
	if (fd < 0)
		perror_msg_and_fail("open: %s", fd_path);
	char *fd_str = xasprintf("%d%s%s%s", fd,
				 DECODE_FDS ? "<" : "",
				 DECODE_FDS ? fd_path : "",
				 DECODE_FDS ? ">" : "");

	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, fmt);

	k_quotactl_fd(-1, QCMD(Q_GETFMT, GRPQUOTA), -2, fmt);
#ifndef PATH_TRACING
	printf("quotactl_fd(-1, QCMD(Q_GETFMT, GRPQUOTA), %p) = %s\n",
	       fmt, errstr);
#endif

	long rc = k_quotactl_fd(fd, QCMD(Q_GETFMT, USRQUOTA), -3, fmt);
	printf("quotactl_fd(%s, QCMD(Q_GETFMT, USRQUOTA), ", fd_str);
	if (rc) {
		printf("%p", fmt);
	} else {
		printf("[");
		printxval(quota_formats, *fmt, "QFMT_VFS_???");
		printf("]");
	}
	printf(") = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
