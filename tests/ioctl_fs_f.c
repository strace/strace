/*
 * Check decoding of FS_IOC{,32}_{G,S}ETFLAGS ioctl commands.
 *
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <linux/fs.h>

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>

static const char *errstr;

static int
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(-1, cmd, arg);
	errstr = sprintrc(rc);

	return rc;
}

static int
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

int
main(int argc, const char *argv[])
{
	static const struct {
		uint32_t cmd;
		const char *str;
		bool on_enter;
		bool on_exit;
		bool skip;
	} cmds[] = {
		{ ARG_STR(FS_IOC32_GETFLAGS), false, true, false },
		{ ARG_STR(FS_IOC32_SETFLAGS), true, false, false },
		{ ARG_STR(FS_IOC_GETFLAGS), false, true,
		  FS_IOC_GETFLAGS == FS_IOC32_GETFLAGS },
		{ ARG_STR(FS_IOC_SETFLAGS), true, false,
		  FS_IOC_SETFLAGS == FS_IOC32_SETFLAGS },
		{ _IO('f', 0xff), "_IOC(_IOC_NONE, 0x66, 0xff, 0)",
		  false, false, false },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, p_flags);

	for (size_t i = 0; i < ARRAY_SIZE(cmds); ++i) {
		if (cmds[i].skip)
			continue;

		do_ioctl(cmds[i].cmd, 0);
		printf("ioctl(-1, " XLAT_FMT ", %s) = %s\n",
		       XLAT_SEL(cmds[i].cmd, cmds[i].str),
		       (cmds[i].on_enter || cmds[i].on_exit) ? "NULL" : "0",
		       errstr);

		do_ioctl_ptr(cmds[i].cmd, p_flags + 1);
		printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
		       XLAT_SEL(cmds[i].cmd, cmds[i].str),
		       p_flags + 1, errstr);

#define VALID_FLAGS 0xf2ffffff
#define INVALID_FLAGS  0xd000000
		*p_flags = INVALID_FLAGS;

		if (cmds[i].on_enter) {
			do_ioctl_ptr(cmds[i].cmd, p_flags);
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = %s\n",
			       XLAT_SEL(cmds[i].cmd, cmds[i].str),
			       XLAT_UNKNOWN(INVALID_FLAGS, "FS_???_FL"),
			       errstr);

			*p_flags = ~*p_flags;
			do_ioctl_ptr(cmds[i].cmd, p_flags);
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = %s\n",
			       XLAT_SEL(cmds[i].cmd, cmds[i].str),
			       XLAT_KNOWN(VALID_FLAGS,
					  "FS_SECRM_FL|"
					  "FS_UNRM_FL|"
					  "FS_COMPR_FL|"
					  "FS_SYNC_FL|"
					  "FS_IMMUTABLE_FL|"
					  "FS_APPEND_FL|"
					  "FS_NODUMP_FL|"
					  "FS_NOATIME_FL|"
					  "FS_DIRTY_FL|"
					  "FS_COMPRBLK_FL|"
					  "FS_NOCOMP_FL|"
					  "FS_ENCRYPT_FL|"
					  "FS_INDEX_FL|"
					  "FS_IMAGIC_FL|"
					  "FS_JOURNAL_DATA_FL|"
					  "FS_NOTAIL_FL|"
					  "FS_DIRSYNC_FL|"
					  "FS_TOPDIR_FL|"
					  "FS_HUGE_FILE_FL|"
					  "FS_EXTENT_FL|"
					  "FS_VERITY_FL|"
					  "FS_EA_INODE_FL|"
					  "FS_EOFBLOCKS_FL|"
					  "FS_NOCOW_FL|"
					  "FS_DAX_FL|"
					  "FS_INLINE_DATA_FL|"
					  "FS_PROJINHERIT_FL|"
					  "FS_CASEFOLD_FL|"
					  "FS_RESERVED_FL"),
			       errstr);
		} else if (cmds[i].on_exit) {
			do_ioctl_ptr(cmds[i].cmd, p_flags);
			printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
			       XLAT_SEL(cmds[i].cmd, cmds[i].str),
			       p_flags, errstr);
		}

	}

	puts("+++ exited with 0 +++");
	return 0;
}
