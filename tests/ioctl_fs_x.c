/*
 * Check decoding of linux/fs.h 'X' ioctl commands.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <linux/fs.h>

#ifdef FIFREEZE
# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ioctl.h>

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
main(void)
{
	static const struct {
		uint32_t cmd;
		const char *str;
		bool has_arg;
	} simple_cmds[] = {
		{ ARG_STR(FIFREEZE), false },
		{ ARG_STR(FITHAW), false },
# ifdef FS_IOC_FSGETXATTR
		{ ARG_STR(FS_IOC_FSGETXATTR), true },
# endif
# ifdef FS_IOC_FSSETXATTR
		{ ARG_STR(FS_IOC_FSSETXATTR), true },
# endif
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

# if defined FITRIM
	static const struct {
		uint32_t cmd;
		const char *str;
	} null_arg_cmds[] = {
		{ ARG_STR(FITRIM) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(null_arg_cmds); ++i) {
		do_ioctl(null_arg_cmds[i].cmd, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s\n",
		       XLAT_SEL(null_arg_cmds[i].cmd, null_arg_cmds[i].str),
		       errstr);
# endif /* FITRIM */

# ifdef FITRIM
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
# endif /* FITRIM */

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("FIFREEZE")

#endif
