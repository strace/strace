/*
 * Check decoding of linux/fs.h 'X' ioctl commands.
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
#include <stdlib.h>
#include <sys/ioctl.h>

static const char *errstr;

static int
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(-1, cmd, arg);
	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Return value [%d] does not match"
				   " expectations [%d]", rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static int
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

#ifdef INJECT_RETVAL
static void
skip_ioctls(int argc, const char *argv[])
{
	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; ++i) {
		int rc = ioctl(-1, FIFREEZE, 0);

		printf("ioctl(-1, %s) = %s%s\n",
		       XLAT_STR(FIFREEZE), sprintrc(rc),
		       rc == INJECT_RETVAL ? " (INJECTED)" : "");

		if (rc == INJECT_RETVAL)
			return;
	}

	error_msg_and_fail("Issued %lu ioctl syscalls but failed"
			   " to detect an injected return code %d",
			   num_skip, INJECT_RETVAL);
}
#endif /* INJECT_RETVAL */

int
main(int argc, const char *argv[])
{
#ifdef INJECT_RETVAL
	skip_ioctls(argc, argv);
#endif

	static const struct {
		uint32_t cmd;
		const char *str;
		bool has_arg;
	} simple_cmds[] = {
		{ ARG_STR(FIFREEZE), false },
		{ ARG_STR(FITHAW), false },
		{ _IO('X', 0xff), "_IOC(_IOC_NONE, 0x58, 0xff, 0)", true },
	};

	for (size_t i = 0; i < ARRAY_SIZE(simple_cmds); ++i) {
		do_ioctl(simple_cmds[i].cmd, 0);
		if (simple_cmds[i].has_arg) {
			printf("ioctl(-1, " XLAT_FMT ", 0) = %s\n",
			       XLAT_SEL(simple_cmds[i].cmd, simple_cmds[i].str),
			       errstr);
		} else {
			printf("ioctl(-1, " XLAT_FMT ") = %s\n",
			       XLAT_SEL(simple_cmds[i].cmd, simple_cmds[i].str),
			       errstr);
		}

		static const unsigned long arg =
			(unsigned long) 0xbadc0deddeadc0deULL;

		do_ioctl(simple_cmds[i].cmd, arg);
		if (simple_cmds[i].has_arg) {
			printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s\n",
			       XLAT_SEL(simple_cmds[i].cmd, simple_cmds[i].str),
			       arg, errstr);
		} else {
			printf("ioctl(-1, " XLAT_FMT ") = %s\n",
			       XLAT_SEL(simple_cmds[i].cmd, simple_cmds[i].str),
			       errstr);
		}
	}

	static const struct {
		uint32_t cmd;
		const char *str;
	} null_arg_cmds[] = {
		{ ARG_STR(FITRIM) },
		{ ARG_STR(FS_IOC_FSSETXATTR) },
		{ ARG_STR(FS_IOC_FSGETXATTR) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(null_arg_cmds); ++i) {
		do_ioctl(null_arg_cmds[i].cmd, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s\n",
		       XLAT_SEL(null_arg_cmds[i].cmd, null_arg_cmds[i].str),
		       errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct fstrim_range, p_range);

	do_ioctl_ptr(FITRIM, (char *) p_range + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(FITRIM), (char *) p_range + 1, errstr);

	p_range->start = (typeof(p_range->start)) 0xdeadbeefcafef00dULL;
	p_range->len = (typeof(p_range->len)) 0xfacefeedbabec0deULL;
	p_range->minlen = (typeof(p_range->minlen)) 0xbadc0deddeadc0deULL;

	do_ioctl_ptr(FITRIM, p_range);
	printf("ioctl(-1, %s, {start=%#jx, len=%ju, minlen=%ju}) = %s\n",
	       XLAT_STR(FITRIM), (uintmax_t) p_range->start,
	       (uintmax_t) p_range->len, (uintmax_t) p_range->minlen,
	       errstr);

	/* FS_IOC_FSSETXATTR */
	TAIL_ALLOC_OBJECT_CONST_PTR(struct fsxattr, p_fsxattr);

	do_ioctl_ptr(FS_IOC_FSSETXATTR, (char *) p_fsxattr + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(FS_IOC_FSSETXATTR), (char *) p_fsxattr + 1, errstr);

#define VALID_FSX_XFLAGS 0x8001fffb
#define INVALID_FSX_XFLAGS 0x7ffe0004

	p_fsxattr->fsx_xflags = VALID_FSX_XFLAGS;
	p_fsxattr->fsx_extsize = 0xdeadbeefU;
	p_fsxattr->fsx_projid = 0xcafef00dU;
	p_fsxattr->fsx_cowextsize = 0xbabec0deU;

	do_ioctl_ptr(FS_IOC_FSSETXATTR, p_fsxattr);
	printf("ioctl(-1, %s, {fsx_xflags=%s, fsx_extsize=%u, fsx_projid=%#x"
	       ", fsx_cowextsize=%u}) = %s\n",
	       XLAT_STR(FS_IOC_FSSETXATTR),
	       XLAT_KNOWN(VALID_FSX_XFLAGS,
			  "FS_XFLAG_REALTIME|FS_XFLAG_PREALLOC|"
			  "FS_XFLAG_IMMUTABLE|FS_XFLAG_APPEND|"
			  "FS_XFLAG_SYNC|FS_XFLAG_NOATIME|"
			  "FS_XFLAG_NODUMP|FS_XFLAG_RTINHERIT|"
			  "FS_XFLAG_PROJINHERIT|FS_XFLAG_NOSYMLINKS|"
			  "FS_XFLAG_EXTSIZE|FS_XFLAG_EXTSZINHERIT|"
			  "FS_XFLAG_NODEFRAG|FS_XFLAG_FILESTREAM|"
			  "FS_XFLAG_DAX|FS_XFLAG_COWEXTSIZE|FS_XFLAG_HASATTR"),
	       p_fsxattr->fsx_extsize,
	       p_fsxattr->fsx_projid,
	       p_fsxattr->fsx_cowextsize,
	       errstr);

	p_fsxattr->fsx_xflags = ~p_fsxattr->fsx_xflags;

	do_ioctl_ptr(FS_IOC_FSSETXATTR, p_fsxattr);
	printf("ioctl(-1, %s, {fsx_xflags=%s, fsx_extsize=%u, fsx_projid=%#x"
	       ", fsx_cowextsize=%u}) = %s\n",
	       XLAT_STR(FS_IOC_FSSETXATTR),
	       XLAT_UNKNOWN(INVALID_FSX_XFLAGS, "FS_XFLAG_???"),
	       p_fsxattr->fsx_extsize,
	       p_fsxattr->fsx_projid,
	       p_fsxattr->fsx_cowextsize,
	       errstr);

	/* FS_IOC_FSGETXATTR */
	do_ioctl_ptr(FS_IOC_FSGETXATTR, (char *) p_fsxattr + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(FS_IOC_FSGETXATTR), (char *) p_fsxattr + 1, errstr);

	p_fsxattr->fsx_xflags = VALID_FSX_XFLAGS;
	p_fsxattr->fsx_nextents = 0xfacefeedU;

	if (do_ioctl_ptr(FS_IOC_FSGETXATTR, p_fsxattr) < 0) {
		printf("ioctl(-1, %s, %p) = %s\n",
		       XLAT_STR(FS_IOC_FSGETXATTR), p_fsxattr, errstr);
	} else {
		printf("ioctl(-1, %s, {fsx_xflags=%s, fsx_extsize=%u"
		       ", fsx_nextents=%u, fsx_projid=%#x"
		       ", fsx_cowextsize=%u}) = %s\n",
		       XLAT_STR(FS_IOC_FSGETXATTR),
		       XLAT_KNOWN(VALID_FSX_XFLAGS,
				  "FS_XFLAG_REALTIME|FS_XFLAG_PREALLOC|"
				  "FS_XFLAG_IMMUTABLE|FS_XFLAG_APPEND|"
				  "FS_XFLAG_SYNC|FS_XFLAG_NOATIME|"
				  "FS_XFLAG_NODUMP|FS_XFLAG_RTINHERIT|"
				  "FS_XFLAG_PROJINHERIT|FS_XFLAG_NOSYMLINKS|"
				  "FS_XFLAG_EXTSIZE|FS_XFLAG_EXTSZINHERIT|"
				  "FS_XFLAG_NODEFRAG|FS_XFLAG_FILESTREAM|"
				  "FS_XFLAG_DAX|FS_XFLAG_COWEXTSIZE|FS_XFLAG_HASATTR"),
		       p_fsxattr->fsx_extsize,
		       p_fsxattr->fsx_nextents,
		       p_fsxattr->fsx_projid,
		       p_fsxattr->fsx_cowextsize,
		       errstr);
	}

	p_fsxattr->fsx_xflags = ~p_fsxattr->fsx_xflags;

	if (do_ioctl_ptr(FS_IOC_FSGETXATTR, p_fsxattr) < 0) {
		printf("ioctl(-1, %s, %p) = %s\n",
		       XLAT_STR(FS_IOC_FSGETXATTR), p_fsxattr, errstr);
	} else {
		printf("ioctl(-1, %s, {fsx_xflags=%s, fsx_extsize=%u"
		       ", fsx_nextents=%u, fsx_projid=%#x"
		       ", fsx_cowextsize=%u}) = %s\n",
		       XLAT_STR(FS_IOC_FSGETXATTR),
		       XLAT_UNKNOWN(INVALID_FSX_XFLAGS, "FS_XFLAG_???"),
		       p_fsxattr->fsx_extsize,
		       p_fsxattr->fsx_nextents,
		       p_fsxattr->fsx_projid,
		       p_fsxattr->fsx_cowextsize,
		       errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
