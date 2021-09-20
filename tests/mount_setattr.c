/*
 * Check decoding of mount_setattr syscall.
 *
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <linux/fcntl.h>
#include <linux/mount.h>

static const char *rcstr;

static long
k_mount_setattr(const unsigned int dfd,
		const void *fname,
		const unsigned int flags,
		const void *attr,
		kernel_ulong_t size)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | dfd;
	const kernel_ulong_t arg2 = (uintptr_t) fname;
	const kernel_ulong_t arg3 = fill | flags;
	const kernel_ulong_t arg4 = (uintptr_t) attr;
	const kernel_ulong_t arg5 = size;
	const long rc =
		syscall(__NR_mount_setattr, arg1, arg2, arg3, arg4, arg5, bad);
	rcstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

#ifndef PATH_TRACING
	char *cwd = get_fd_path(get_dir_fd("."));
#endif
	static const char path_full[] = "/dev/full";
	const char *const path = tail_memdup(path_full, sizeof(path_full));
	char *const fname = tail_alloc(PATH_MAX);
	const void *const efault = fname + PATH_MAX;
	const char *const empty = efault - 1;
	fill_memory_ex(fname, PATH_MAX, '0', 10);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct mount_attr, attr);
	struct mount_attr *const attr_big = tail_alloc(sizeof(*attr_big) + 8);
	static const struct strval32 valid_flags =
		{ ARG_STR(AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE) };
        const unsigned int dfd = 9;

	k_mount_setattr(-1, 0, AT_SYMLINK_NOFOLLOW, efault, -1U);
#ifndef PATH_TRACING
	printf("mount_setattr(-1, NULL, %s, %p, %u) = %s\n",
	       "AT_SYMLINK_NOFOLLOW", efault, -1U, rcstr);
#endif

	k_mount_setattr(-100, fname, 0, attr, MOUNT_ATTR_SIZE_VER0 - 1);
#ifndef PATH_TRACING
	printf("mount_setattr(AT_FDCWD<%s>, \"%.*s\"..., 0, %p, %u) = %s\n",
	       cwd, (int) PATH_MAX - 1, fname,
	       attr, MOUNT_ATTR_SIZE_VER0 - 1, rcstr);
#endif

	fname[PATH_MAX - 1] = '\0';
	k_mount_setattr(dfd, fname, -1U,
			1 + (void *) attr, MOUNT_ATTR_SIZE_VER0);
	printf("mount_setattr(%d<%s>, \"%s\", %s|%#x, %p, %u) = %s\n",
	       dfd, path, fname, valid_flags.str, ~valid_flags.val,
	       1 + (void *) attr, MOUNT_ATTR_SIZE_VER0, rcstr);

	k_mount_setattr(-1, efault, valid_flags.val,
			1 + (void *) attr, MOUNT_ATTR_SIZE_VER0 - 1);
#ifndef PATH_TRACING
	printf("mount_setattr(-1, %p, %s, %p, %u) = %s\n",
	       efault, valid_flags.str,
	       1 + (void *) attr, MOUNT_ATTR_SIZE_VER0 - 1, rcstr);
#endif

	k_mount_setattr(-1, empty, ~valid_flags.val, 0, MOUNT_ATTR_SIZE_VER0);
#ifndef PATH_TRACING
	printf("mount_setattr(-1, \"\", %#x /* AT_??? */, NULL, %u) = %s\n",
	       ~valid_flags.val, MOUNT_ATTR_SIZE_VER0, rcstr);
#endif

	static const struct strval64 valid_attr =
		{ ARG_STR(MOUNT_ATTR_RDONLY|MOUNT_ATTR_NOSUID|MOUNT_ATTR_NODEV|MOUNT_ATTR_NOEXEC|MOUNT_ATTR_NOATIME|MOUNT_ATTR_STRICTATIME|MOUNT_ATTR_NODIRATIME|MOUNT_ATTR_IDMAP|MOUNT_ATTR_NOSYMFOLLOW) };

	for (unsigned int j = 0; j < 4; ++j) {
		struct mount_attr *const a = j > 1 ? attr_big : attr;
		const size_t size = j ? sizeof(*a) + 8 : sizeof(*a);

		if (j == 3)
			memset(attr_big + 1, 0, 8);
		else
			fill_memory(attr_big + 1, 8);

		a->attr_set = 0xffffffff00000000ULL;
		a->attr_clr = 0;
		a->propagation = MS_UNBINDABLE;
		a->userns_fd = dfd;

		k_mount_setattr(-1, path, 0, a, size);
		printf("mount_setattr(-1, \"%s\", 0"
		       ", {attr_set=0xffffffff00000000 /* MOUNT_ATTR_??? */"
		       ", attr_clr=0, propagation=%s, userns_fd=%u",
		       path, "MS_UNBINDABLE", dfd);
		if (j == 1)
			printf(", ???");
		if (j == 2) {
			printf(", /* bytes %zu..%zu */ \"\\x80\\x81"
			       "\\x82\\x83\\x84\\x85\\x86\\x87\"",
			       sizeof(*a), sizeof(*a) + 7);
		}
		printf("}, %zu) = %s\n", size, rcstr);

		a->attr_set = valid_attr.val;
		a->attr_clr = ~valid_attr.val;
		a->propagation = MS_PRIVATE | MS_SHARED;
		a->userns_fd = dfd;

		k_mount_setattr(-1, path, 0, a, size);
		printf("mount_setattr(-1, \"%s\", 0"
		       ", {attr_set=%s, attr_clr=%#llx /* MOUNT_ATTR_??? */"
		       ", propagation=%#x /* MS_??? */, userns_fd=%d<%s>",
		       path, valid_attr.str, (unsigned long long) a->attr_clr,
		       MS_PRIVATE | MS_SHARED, dfd, path);
		if (j == 1)
			printf(", ???");
		if (j == 2) {
			printf(", /* bytes %zu..%zu */ \"\\x80\\x81"
			       "\\x82\\x83\\x84\\x85\\x86\\x87\"",
			       sizeof(*a), sizeof(*a) + 7);
		}
		printf("}, %zu) = %s\n", size, rcstr);

		a->attr_set = MOUNT_ATTR_NOSUID;
		a->attr_clr = MOUNT_ATTR_NODEV;
		a->propagation = MS_SLAVE;
		a->userns_fd = 0xdefaced00000000ULL | dfd;

		k_mount_setattr(dfd, empty, AT_EMPTY_PATH, a, size);
		printf("mount_setattr(%d<%s>, \"\", %s, {attr_set=%s, attr_clr=%s"
		       ", propagation=%s, userns_fd=%llu",
		       dfd, path, "AT_EMPTY_PATH",
		       "MOUNT_ATTR_NOSUID", "MOUNT_ATTR_NODEV", "MS_SLAVE",
		       (unsigned long long) a->userns_fd);
		if (j == 1)
			printf(", ???");
		if (j == 2) {
			printf(", /* bytes %zu..%zu */ \"\\x80\\x81"
			       "\\x82\\x83\\x84\\x85\\x86\\x87\"",
			       sizeof(*a), sizeof(*a) + 7);
		}
		printf("}, %zu) = %s\n", size, rcstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
