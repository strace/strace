/*
 * Check decoding of move_mount syscall.
 *
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static const char *errstr;

static long
k_move_mount(const unsigned int from_dfd, const void *from_fname,
	     const unsigned int to_dfd, const void *to_fname,
	     const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | from_dfd;
	const kernel_ulong_t arg2 = (uintptr_t) from_fname;
	const kernel_ulong_t arg3 = fill | to_dfd;
	const kernel_ulong_t arg4 = (uintptr_t) to_fname;
	const kernel_ulong_t arg5 = fill | flags;
	const long rc = syscall(__NR_move_mount,
				arg1, arg2, arg3, arg4, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	char *cwd = get_fd_path(get_dir_fd("."));
	static const char path_full[] = "/dev/full";
	const char *const path = tail_memdup(path_full, sizeof(path_full));
	const void *const efault = path + sizeof(path_full);
	const char *const empty = efault - 1;
	char *const fname = tail_alloc(PATH_MAX);
	fill_memory_ex(fname, PATH_MAX, '0', 10);

        int dfd = open(path, O_WRONLY);
        if (dfd < 0)
                perror_msg_and_fail("open: %s", path);

	k_move_mount(-1, 0, -100, efault, 0);
#ifndef PATH_TRACING
	printf("move_mount(-1, NULL, AT_FDCWD<%s>, %p, 0) = %s\n",
	       cwd, efault, errstr);
#endif

	k_move_mount(-100, efault, -1, 0, 0);
#ifndef PATH_TRACING
	printf("move_mount(AT_FDCWD<%s>, %p, -1, NULL, 0) = %s\n",
	       cwd, efault, errstr);
#endif

	k_move_mount(dfd, fname, -100, empty, 1);
	printf("move_mount(%d<%s>, \"%.*s\"..., AT_FDCWD<%s>, \"\", %s) = %s\n",
	       dfd, path, (int) PATH_MAX - 1, fname, cwd,
	       "MOVE_MOUNT_F_SYMLINKS", errstr);

	k_move_mount(-100, empty, dfd, fname, 0x10);
	printf("move_mount(AT_FDCWD<%s>, \"\", %d<%s>, \"%.*s\"..., %s) = %s\n",
	       cwd, dfd, path, (int) PATH_MAX - 1, fname,
	       "MOVE_MOUNT_T_SYMLINKS", errstr);

	k_move_mount(-100, empty, dfd, fname, 0x100);
	printf("move_mount(AT_FDCWD<%s>, \"\", %d<%s>, \"%.*s\"..., %s) = %s\n",
	       cwd, dfd, path, (int) PATH_MAX - 1, fname,
	       "MOVE_MOUNT_SET_GROUP", errstr);

#define f_flags_str "MOVE_MOUNT_F_SYMLINKS|MOVE_MOUNT_F_AUTOMOUNTS|MOVE_MOUNT_F_EMPTY_PATH"
	fname[PATH_MAX - 1] = '\0';
	k_move_mount(dfd, fname, -100, empty, 7);
	printf("move_mount(%d<%s>, \"%s\", AT_FDCWD<%s>, \"\", %s) = %s\n",
	       dfd, path, fname, cwd, f_flags_str, errstr);

#define t_flags_str "MOVE_MOUNT_T_SYMLINKS|MOVE_MOUNT_T_AUTOMOUNTS|MOVE_MOUNT_T_EMPTY_PATH"
	k_move_mount(-100, empty, dfd, fname, 0x70);
	printf("move_mount(AT_FDCWD<%s>, \"\", %d<%s>, \"%s\", %s) = %s\n",
	       cwd, dfd, path, fname, t_flags_str, errstr);

#define set_group_str "MOVE_MOUNT_SET_GROUP"
	k_move_mount(-100, empty, dfd, fname, 0x100);
	printf("move_mount(AT_FDCWD<%s>, \"\", %d<%s>, \"%s\", %s) = %s\n",
	       cwd, dfd, path, fname, set_group_str, errstr);

	k_move_mount(-1, path, -100, empty, 0x177);
	printf("move_mount(-1, \"%s\", AT_FDCWD<%s>, \"\", %s) = %s\n",
	       path, cwd, f_flags_str "|" t_flags_str "|" set_group_str,
	       errstr);

	k_move_mount(-100, empty, -1, path, -1);
	printf("move_mount(AT_FDCWD<%s>, \"\", -1, \"%s\", %s) = %s\n",
	       cwd, path,
	       f_flags_str "|" t_flags_str "|" set_group_str "|0xfffffe88",
	       errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
