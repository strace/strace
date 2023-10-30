/*
 * This file is part of ioctl_inotify strace test.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "scno.h"
#include <linux/ioctl.h>

#ifndef INOTIFY_IOC_SETNEXTWD
# define INOTIFY_IOC_SETNEXTWD  _IOW('I', 0, int32_t)
#endif

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	return syscall(__NR_ioctl, fd, cmd, arg);
}

int
main(void)
{
	static const kernel_ulong_t unknown_inotify_cmd =
		(kernel_ulong_t) 0xbadc0dedfeed49edULL;
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;

	/* Unknown inotify commands */
	sys_ioctl(-1, unknown_inotify_cmd, magic);
	printf("ioctl(-1, _IOC(%s_IOC_READ|_IOC_WRITE, 0x49, %#x, %#x), "
	       "%#lx)" RVAL_EBADF,
	       _IOC_DIR((unsigned int) unknown_inotify_cmd) & _IOC_NONE ?
	       "_IOC_NONE|" : "",
	       _IOC_NR((unsigned int) unknown_inotify_cmd),
	       _IOC_SIZE((unsigned int) unknown_inotify_cmd),
	       (unsigned long) magic);

	sys_ioctl(-1, INOTIFY_IOC_SETNEXTWD + 1, magic);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0x49, %#x, %#x), %#lx)" RVAL_EBADF,
	       (unsigned int) _IOC_NR(INOTIFY_IOC_SETNEXTWD + 1),
	       (unsigned int) _IOC_SIZE(INOTIFY_IOC_SETNEXTWD + 1),
	       (unsigned long) magic);

	/* INOTIFY_IOC_SETNEXTWD */
	sys_ioctl(-1, INOTIFY_IOC_SETNEXTWD, magic);
	printf("ioctl(-1, INOTIFY_IOC_SETNEXTWD, %d)" RVAL_EBADF,
	       (int) magic);

	puts("+++ exited with 0 +++");
	return 0;
}
