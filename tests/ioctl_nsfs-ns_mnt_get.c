/*
 * Check decoding of NS_MNT_GET_{INFO,NEXT,PREV} commands of ioctl syscall.
 *
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/nsfs.h>

int
main(void)
{
	static const char mnt_path[] = "/proc/self/ns/mnt";
	static const struct strval32 ops[] = {
		{ ARG_STR(NS_MNT_GET_INFO) },
		{ ARG_STR(NS_MNT_GET_NEXT) },
		{ ARG_STR(NS_MNT_GET_PREV) },
		{ ARG_STR(_IOC(_IOC_READ, 0xb7, 0xff, 0x10)) },

	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct mnt_ns_info, info);
	int fd = open(mnt_path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_skip("open: %s", mnt_path);

	for (size_t i = 0; i < ARRAY_SIZE(ops); ++i) {
		int rc = ioctl(-1, ops[i].val, info);
		printf("ioctl(-1, %s, %p) = %s\n",
		       ops[i].str, info, sprintrc(rc));

		rc = ioctl(fd, ops[i].val, info);
		printf("ioctl(%d, %s, ", fd, ops[i].str);
		if (rc < 0) {
			printf("%p) = %s\n",
			       info, sprintrc(rc));
		} else {
			printf("{size=%u, nr_mounts=%u, mnt_ns_id=%#llx})"
			       " = %d\n",
			       info->size, info->nr_mounts,
			       (unsigned long long) info->mnt_ns_id, rc);
		}

		int code = _IOC(_IOC_READ, NSIO, _IOC_NR(ops[i].val),
				MNT_NS_INFO_SIZE_VER0 - 1);
		rc = ioctl(fd, code, info);
		printf("ioctl(%d, _IOC(_IOC_READ, %#x, %#x, %#x), %p) = %s\n",
		       fd, NSIO, _IOC_NR(code), _IOC_SIZE(code),
		       info, sprintrc(rc));

		code = _IOC(_IOC_READ, NSIO, _IOC_NR(ops[i].val),
			    sizeof(*info) + 1);
		rc = ioctl(fd, code, info);
		printf("ioctl(%d, _IOC(_IOC_READ, %#x, %#x, %#x), ",
		       fd, NSIO, _IOC_NR(code), _IOC_SIZE(code));
		if (rc < 0) {
			printf("%p) = %s\n",
			       info, sprintrc(rc));
		} else {
			printf("{size=%u, nr_mounts=%u, mnt_ns_id=%#llx})"
			       " = %d\n",
			       info->size, info->nr_mounts,
			       (unsigned long long) info->mnt_ns_id, rc);
		}
	}

	close(fd);

	puts("+++ exited with 0 +++");
	return 0;
}
