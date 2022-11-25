/*
 * Check decoding of ioctl TIOCM* commands.
 *
 * Copyright (c) 2020-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
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
	} cmds[] = {
		{ ARG_STR(TIOCMGET), false, true },
		{ ARG_STR(TIOCMBIS), true, false },
		{ ARG_STR(TIOCMBIC), true, false },
		{ ARG_STR(TIOCMSET), true, false },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, p_flags);
	const void *const efault = p_flags + 1;

	for (size_t i = 0; i < ARRAY_SIZE(cmds); ++i) {
		do_ioctl(cmds[i].cmd, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s\n",
		       XLAT_SEL(cmds[i].cmd, cmds[i].str), errstr);

		do_ioctl_ptr(cmds[i].cmd, efault);
		printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
		       XLAT_SEL(cmds[i].cmd, cmds[i].str),
		       efault, errstr);

#ifdef __mips__
# define VALID_FLAGS 0xe777
# define INVALID_FLAGS  0xffff1888
#else
# define VALID_FLAGS 0xe1ff
# define INVALID_FLAGS  0xffff1e00
#endif
		*p_flags = INVALID_FLAGS;

		if (cmds[i].on_enter) {
			do_ioctl_ptr(cmds[i].cmd, p_flags);
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = %s\n",
			       XLAT_SEL(cmds[i].cmd, cmds[i].str),
			       XLAT_UNKNOWN(INVALID_FLAGS, "TIOCM_???"),
			       errstr);

			*p_flags = ~*p_flags;
			do_ioctl_ptr(cmds[i].cmd, p_flags);
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = %s\n",
			       XLAT_SEL(cmds[i].cmd, cmds[i].str),
			       XLAT_KNOWN(VALID_FLAGS,
					  "TIOCM_LE|"
					  "TIOCM_DTR|"
					  "TIOCM_RTS|"
					  "TIOCM_ST|"
					  "TIOCM_SR|"
					  "TIOCM_CTS|"
					  "TIOCM_CAR|"
					  "TIOCM_RNG|"
					  "TIOCM_DSR|"
					  "TIOCM_OUT1|"
					  "TIOCM_OUT2|"
					  "TIOCM_LOOP"),
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
