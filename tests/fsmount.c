/*
 * Check decoding of fsmount syscall.
 *
 * Copyright (c) 2019-2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static const char *errstr;

static long
k_fsmount(const unsigned int fs_fd,
	  const unsigned int flags,
	  const unsigned int attr_flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fs_fd;
	const kernel_ulong_t arg2 = fill | flags;
	const kernel_ulong_t arg3 = fill | attr_flags;
	const long rc = syscall(__NR_fsmount,
				arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const char path[] = "/dev/full";
        int fd = open(path, O_WRONLY);
        if (fd < 0)
                perror_msg_and_fail("open: %s", path);

	static const struct strval32 flags[] = {
		{ ARG_STR(0) },
		{ 1, "FSMOUNT_CLOEXEC" },
		{ 2, "0x2 /* FSMOUNT_??? */" },
		{ 0xfffffffe, "0xfffffffe /* FSMOUNT_??? */" },
		{ -1, "FSMOUNT_CLOEXEC|0xfffffffe" }
	},
	attrs[] = {
		{ ARG_STR(0) },
		{ 1, "MOUNT_ATTR_RDONLY" },
		{ 0x10, "MOUNT_ATTR_NOATIME" },
		{ 0x2000bf,
		  "MOUNT_ATTR_RDONLY|MOUNT_ATTR_NOSUID|MOUNT_ATTR_NODEV"
		  "|MOUNT_ATTR_NOEXEC|MOUNT_ATTR_NOATIME|MOUNT_ATTR_STRICTATIME"
		  "|MOUNT_ATTR_NODIRATIME|MOUNT_ATTR_NOSYMFOLLOW" },
		{ 0x2000ff,
		  "MOUNT_ATTR_RDONLY|MOUNT_ATTR_NOSUID|MOUNT_ATTR_NODEV"
		  "|MOUNT_ATTR_NOEXEC|MOUNT_ATTR__ATIME|MOUNT_ATTR_NODIRATIME"
		  "|MOUNT_ATTR_NOSYMFOLLOW" },
		{ 0x40, "0x40 /* MOUNT_ATTR_??? */" },
		{ 0xffdfff40, "0xffdfff40 /* MOUNT_ATTR_??? */" },
		{ -1,
		  "MOUNT_ATTR_RDONLY|MOUNT_ATTR_NOSUID|MOUNT_ATTR_NODEV"
		  "|MOUNT_ATTR_NOEXEC|MOUNT_ATTR__ATIME|MOUNT_ATTR_NODIRATIME"
		  "|MOUNT_ATTR_NOSYMFOLLOW|0xffdfff00" }
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(flags); ++i) {
		for (unsigned int j = 0; j < ARRAY_SIZE(attrs); ++j) {
			k_fsmount(-1, flags[i].val, attrs[j].val);
			printf("fsmount(-1, %s, %s) = %s\n",
			       flags[i].str, attrs[j].str, errstr);

			k_fsmount(-100, flags[i].val, attrs[j].val);
			printf("fsmount(-100, %s, %s) = %s\n",
			       flags[i].str, attrs[j].str, errstr);

			k_fsmount(fd, flags[i].val, attrs[j].val);
			printf("fsmount(%d<%s>, %s, %s) = %s\n",
			       fd, path, flags[i].str, attrs[j].str, errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
