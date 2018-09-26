/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_syslog

# include <stdio.h>
# include <unistd.h>

# define SYSLOG_ACTION_READ 2
# define SYSLOG_ACTION_SIZE_BUFFER 10

int
main(void)
{
	static const struct cmd_str {
		unsigned int cmd;
		const char *str;
	} no_args[] = {
		{ 0,  "0 /* SYSLOG_ACTION_CLOSE */" },
		{ 1,  "1 /* SYSLOG_ACTION_OPEN */" },
		{ 5,  "5 /* SYSLOG_ACTION_CLEAR */" },
		{ 6,  "6 /* SYSLOG_ACTION_CONSOLE_OFF */" },
		{ 7,  "7 /* SYSLOG_ACTION_CONSOLE_ON */" },
		{ 9,  "9 /* SYSLOG_ACTION_SIZE_UNREAD */" },
		{ 10, "10 /* SYSLOG_ACTION_SIZE_BUFFER */" },
	};
	static const kernel_ulong_t high =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	const long addr = (long) 0xfacefeeddeadbeefULL;
	int rc;
	for (size_t i = 0; i < ARRAY_SIZE(no_args); i++) {
		rc = syscall(__NR_syslog, high | no_args[i].cmd, addr, -1);
		printf("syslog(%s) = %s\n",
		no_args[i].str, sprintrc(rc));
	}

	rc = syscall(__NR_syslog, SYSLOG_ACTION_READ, addr, -1);
	printf("syslog(2 /* SYSLOG_ACTION_READ */, %#lx, -1) = %s\n",
	       addr, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_syslog")

#endif
