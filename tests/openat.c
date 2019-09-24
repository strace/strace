/*
 * Copyright (c) 2016 Katerina Koukiou <k.koukiou@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_openat

# include <asm/fcntl.h>
# include <stdio.h>
# include <unistd.h>

# ifdef O_TMPFILE
/* The kernel & C libraries often inline O_DIRECTORY. */
#  define STRACE_O_TMPFILE (O_TMPFILE & ~O_DIRECTORY)
# else
#  define STRACE_O_TMPFILE 0
# endif

static const char sample[] = "openat.sample";

static void
test_mode_flag(unsigned int mode_val, const char *mode_str,
	       unsigned int flag_val, const char *flag_str)
{
	long rc = syscall(__NR_openat, -1, sample, mode_val | flag_val, 0);
	printf("openat(-1, \"%s\", %s%s%s%s) = %s\n",
	       sample, mode_str,
	       flag_val ? "|" : "", flag_str,
	       flag_val & (O_CREAT | STRACE_O_TMPFILE) ? ", 000" : "",
	       sprintrc(rc));
}

int
main(void)
{
	long fd = syscall(__NR_openat, -100, sample, O_RDONLY|O_CREAT, 0400);
	printf("openat(AT_FDCWD, \"%s\", O_RDONLY|O_CREAT, 0400) = %s\n",
	       sample, sprintrc(fd));

	if (fd != -1) {
		close(fd);
		if (unlink(sample) == -1)
			perror_msg_and_fail("unlink");

		fd = syscall(__NR_openat, -100, sample, O_RDONLY);
		printf("openat(AT_FDCWD, \"%s\", O_RDONLY) = %s\n",
		       sample, sprintrc(fd));
	}

	struct {
		unsigned int val;
		const char *str;
	} modes[] = {
		{ ARG_STR(O_RDONLY) },
		{ ARG_STR(O_WRONLY) },
		{ ARG_STR(O_RDWR) },
		{ ARG_STR(O_ACCMODE) }
	}, flags[] = {
		{ ARG_STR(O_APPEND) },
		{ ARG_STR(O_DIRECT) },
		{ ARG_STR(O_DIRECTORY) },
		{ ARG_STR(O_EXCL) },
		{ ARG_STR(O_LARGEFILE) },
		{ ARG_STR(O_NOATIME) },
		{ ARG_STR(O_NOCTTY) },
		{ ARG_STR(O_NOFOLLOW) },
		{ ARG_STR(O_NONBLOCK) },
		{ ARG_STR(O_SYNC) },
		{ ARG_STR(O_TRUNC) },
		{ ARG_STR(O_CREAT) },
# ifdef O_CLOEXEC
		{ ARG_STR(O_CLOEXEC) },
# endif
# ifdef O_DSYNC
		{ ARG_STR(O_DSYNC) },
# endif
# ifdef __O_SYNC
		{ ARG_STR(__O_SYNC) },
# endif
# ifdef O_PATH
		{ ARG_STR(O_PATH) },
# endif
# ifdef O_TMPFILE
		{ ARG_STR(O_TMPFILE) },
# endif
# ifdef __O_TMPFILE
		{ ARG_STR(__O_TMPFILE) },
# endif
		{ ARG_STR(0x80000000) },
		{ 0, "" }
	};

	for (unsigned int m = 0; m < ARRAY_SIZE(modes); ++m)
		for (unsigned int f = 0; f < ARRAY_SIZE(flags); ++f)
			test_mode_flag(modes[m].val, modes[m].str,
				       flags[f].val, flags[f].str);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_openat")

#endif
