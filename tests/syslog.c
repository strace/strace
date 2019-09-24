/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_syslog

# include <stdio.h>
# include <unistd.h>

# ifdef RETVAL_INJECTED
#  define RET_SFX " (INJECTED)"
# else
#  define RET_SFX ""
# endif

bool
valid_cmd(int cmd)
{
	return cmd >= 0 && cmd <= 10;
}

void
printstr(const char *s, int cmd, long size)
{
	if (size < 0 || !valid_cmd(cmd))
		printf("%p", s);
	else if (size == 0)
		printf("\"\"");
	else if (size <= DEFAULT_STRLEN)
		print_quoted_memory(s, size);
	else {
		print_quoted_memory(s, DEFAULT_STRLEN);
		printf("...");
	}
}

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
	static const struct cmd_str two_args[] = {
		{ 0xfeedbeef, "-17973521 /* SYSLOG_ACTION_??? */" },
		{ -1U, "-1 /* SYSLOG_ACTION_??? */" },
		{ 2,  "2 /* SYSLOG_ACTION_READ */" },
		{ 3,  "3 /* SYSLOG_ACTION_READ_ALL */" },
		{ 4,  "4 /* SYSLOG_ACTION_READ_CLEAR */" },
		{ 11, "11 /* SYSLOG_ACTION_??? */" },
		{ (1U << 31) - 1, "2147483647 /* SYSLOG_ACTION_??? */" },
	};
	static const struct cmd_str levels[] = {
		{ 0xfeedbeef, "-17973521 /* LOGLEVEL_??? */" },
		{ -1U, "-1 /* LOGLEVEL_??? */" },
		{ 0, "0 /* LOGLEVEL_EMERG */" },
		{ 7, "7 /* LOGLEVEL_DEBUG */" },
		{ 8, "8 /* LOGLEVEL_DEBUG+1 */" },
		{ 9, "9 /* LOGLEVEL_??? */" },
		{ (1U << 31) - 1, "2147483647 /* LOGLEVEL_??? */" },
	};
	static const kernel_ulong_t high =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const size_t buf_size = 64;

	const kernel_ulong_t addr = (kernel_ulong_t) 0xfacefeeddeadbeefULL;
	int rc;
	char *buf = tail_alloc(buf_size);

	fill_memory(buf, buf_size);

	for (size_t i = 0; i < ARRAY_SIZE(no_args); i++) {
		rc = syscall(__NR_syslog, high | no_args[i].cmd, addr, -1);
		printf("syslog(%s) = %s" RET_SFX "\n",
		no_args[i].str, sprintrc(rc));
	}

	for (size_t i = 0; i < ARRAY_SIZE(two_args); i++) {
		rc = syscall(__NR_syslog, high | two_args[i].cmd, NULL, -1);
		printf("syslog(%s, NULL, -1) = %s" RET_SFX "\n",
		       two_args[i].str, sprintrc(rc));

# ifdef RETVAL_INJECTED
		/* Avoid valid commands with a bogus address */
		if (!valid_cmd(two_args[i].cmd))
# endif
		{
			rc = syscall(__NR_syslog, high | two_args[i].cmd, addr,
				     -1);
			printf("syslog(%s, %#llx, -1) = %s" RET_SFX "\n",
			       two_args[i].str, (unsigned long long) addr,
			       sprintrc(rc));

			rc = syscall(__NR_syslog, two_args[i].cmd, addr, 0);

			printf("syslog(%s, %s, 0) = %s" RET_SFX "\n",
			       two_args[i].str,
			       !rc && valid_cmd(two_args[i].cmd)
				   && (sizeof(kernel_ulong_t) == sizeof(void *))
				      ? "\"\""
				      : (sizeof(addr) == 8)
					? "0xfacefeeddeadbeef" : "0xdeadbeef",
			       sprintrc(rc));
		}

		rc = syscall(__NR_syslog, two_args[i].cmd, buf, buf_size);
		const char *errstr = sprintrc(rc);

		printf("syslog(%s, ", two_args[i].str);
		if (rc >= 0 && valid_cmd(two_args[i].cmd))
			printstr(buf, two_args[i].cmd, rc);
		else
			printf("%p", buf);
		printf(", %zu) = %s" RET_SFX "\n", buf_size, errstr);
	}

	for (size_t i = 0; i < ARRAY_SIZE(levels); i++) {
		rc = syscall(__NR_syslog, high | 8, addr, levels[i].cmd);
		printf("syslog(8 /* SYSLOG_ACTION_CONSOLE_LEVEL */, %#llx, %s)"
		       " = %s" RET_SFX "\n",
		       (unsigned long long) addr, levels[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_syslog")

#endif
