/*
 * Check printing of character/block device numbers in -yy mode.
 *
 * Copyright (c) 2018-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <unistd.h>

#include "scno.h"

#include <linux/fcntl.h>

#include <sys/sysmacros.h>

#ifndef PRINT_DEVNUM
# define PRINT_DEVNUM 1
#endif

#if PRINT_DEVNUM
# define DEV_FMT "<%s<%s %u:%u>>"
#else
# define DEV_FMT "<%s>"
#endif

#if defined __NR_openat && defined O_PATH

int
main(void)
{
	static const struct {
		const char *path;
		unsigned int major;
		unsigned int minor;
		bool blk;
		bool optional;
	} checks[] = {
		{ "/dev/zero", 1, 5, false, false },
		{ "/dev/full", 1, 7, false, false },
		{ "/dev/sda",  8, 0, true,  true  },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(checks); i++) {
		/*
		 * We can't have nice things here and call openat() directly as
		 * some libcs (yes, musl, I'm looking at you now) are too
		 * frivolous in passing flags to the kernel.
		 */
		long fd = syscall(__NR_openat, AT_FDCWD, checks[i].path,
				  O_RDONLY|O_PATH);

		printf("openat(AT_FDCWD, \"%s\", O_RDONLY|O_PATH) = %s",
		       checks[i].path, sprintrc(fd));
		if (fd >= 0)
			printf(DEV_FMT,
			       checks[i].path
# if PRINT_DEVNUM
			       , checks[i].blk ? "block" : "char",
			       checks[i].major, checks[i].minor
# endif
			       );
		puts("");

		if (fd < 0) {
			if (checks[i].optional)
				continue;
			else
				perror_msg_and_fail("openat(\"%s\")",
						    checks[i].path);
		}

		int rc = fsync(fd);

		printf("fsync(%ld" DEV_FMT ") = %s\n",
		       fd, checks[i].path,
# if PRINT_DEVNUM
		       checks[i].blk ? "block" : "char",
		       checks[i].major, checks[i].minor,
# endif
		       sprintrc(rc));

		close(fd);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_openat && O_PATH")

#endif
