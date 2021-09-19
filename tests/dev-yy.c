/*
 * Check printing of character/block device numbers in -yy mode.
 *
 * Copyright (c) 2018-2021 The strace developers.
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

#ifndef PRINT_PATH
# define PRINT_PATH 1
#endif

#ifndef PRINT_DEVNUM
# if PRINT_PATH
#  define PRINT_DEVNUM 1
# else
#  define PRINT_DEVNUM 0
# endif
#endif

#ifndef PRINT_AT_FDCWD_PATH
# define PRINT_AT_FDCWD_PATH PRINT_DEVNUM
#endif

#if PRINT_DEVNUM
# define DEV_FMT "<%s<%s %u:%u>>"
#elif PRINT_PATH
# define DEV_FMT "<%s>"
#else
# define DEV_FMT ""
#endif

#if defined __NR_openat && defined O_PATH

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");
# if PRINT_AT_FDCWD_PATH
	char *cwd = get_fd_path(get_dir_fd("."));
# endif

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

		printf("openat(AT_FDCWD"
# if PRINT_AT_FDCWD_PATH
		       "<%s>"
# endif
		       ", \"%s\", O_RDONLY|O_PATH) = %s",
# if PRINT_AT_FDCWD_PATH
		       cwd,
# endif
		       checks[i].path, sprintrc(fd));
# if PRINT_PATH
		if (fd >= 0)
			printf(DEV_FMT,
			       checks[i].path
#  if PRINT_DEVNUM
			       , checks[i].blk ? "block" : "char",
			       checks[i].major, checks[i].minor
#  endif
			       );
# endif
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
		       fd,
# if PRINT_PATH
		       checks[i].path,
#  if PRINT_DEVNUM
		       checks[i].blk ? "block" : "char",
		       checks[i].major, checks[i].minor,
#  endif
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
