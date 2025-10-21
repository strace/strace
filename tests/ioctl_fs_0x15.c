/*
 * Check decoding of linux/fs.h 0x15 ioctl commands.
 *
 * Copyright (c) 2020-2025 The strace developers.
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

static const struct strval32 hex_arg_cmds[] = {
	{ _IO(0x15, 0xff), "_IOC(_IOC_NONE, 0x15, 0xff, 0)" },
};

#ifdef INJECT_RETVAL
static void
skip_ioctls(int argc, const char *argv[])
{
	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; ++i) {
		int rc = ioctl(-1, hex_arg_cmds[0].val, 0);

		printf("ioctl(-1, " XLAT_FMT ", 0) = %s%s\n",
		       XLAT_SEL(hex_arg_cmds[0].val, hex_arg_cmds[0].str),
		       sprintrc(rc),
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

	static const unsigned long hex_args[] = {
		0,
		(unsigned long) 0xbadc0deddeadc0deULL,
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(hex_arg_cmds); ++i) {
		for (unsigned int j = 0; j < ARRAY_SIZE(hex_args); ++j) {
			do_ioctl(hex_arg_cmds[i].val, hex_args[j]);
			printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s\n",
			       XLAT_SEL(hex_arg_cmds[i].val, hex_arg_cmds[i].str),
			       hex_args[j], errstr);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
