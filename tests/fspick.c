/*
 * Check decoding of fspick syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fspick

# include <fcntl.h>
# include <limits.h>
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

static const char *errstr;

static long
k_fspick(const unsigned int dfd,
	    const void *fname,
	    const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | dfd;
	const kernel_ulong_t arg2 = (uintptr_t) fname;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_fspick, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	static const char path_full[] = "/dev/full";
	const char *const path = tail_memdup(path_full, sizeof(path_full));
	char *const fname = tail_alloc(PATH_MAX);
	const void *const efault = fname + PATH_MAX;
	const char *const empty = efault - 1;
	fill_memory_ex(fname, PATH_MAX, '0', 10);

        int dfd = open(path, O_WRONLY);
        if (dfd < 0)
                perror_msg_and_fail("open: %s", path);

	k_fspick(-1, 0, 1);
# ifndef PATH_TRACING
	printf("fspick(-1, NULL, %s) = %s\n", "FSPICK_CLOEXEC", errstr);
# endif

	k_fspick(-100, fname, 0);
# ifndef PATH_TRACING
	printf("fspick(%s, \"%.*s\"..., 0) = %s\n",
	       "AT_FDCWD", (int) PATH_MAX - 1, fname, errstr);
# endif

	fname[PATH_MAX - 1] = '\0';
	k_fspick(dfd, fname, 0xfffffff0);
	printf("fspick(%d<%s>, \"%s\", %s) = %s\n",
	       dfd, path, fname, "0xfffffff0 /* FSPICK_??? */", errstr);

	k_fspick(-1, efault, 0xf);
# ifndef PATH_TRACING
	printf("fspick(-1, %p, %s) = %s\n",
	       efault,
	       "FSPICK_CLOEXEC|FSPICK_SYMLINK_NOFOLLOW"
	       "|FSPICK_NO_AUTOMOUNT|FSPICK_EMPTY_PATH",
	       errstr);
# endif

	k_fspick(-1, empty, -1);
# ifndef PATH_TRACING
	printf("fspick(-1, \"\", %s|0xfffffff0) = %s\n",
	       "FSPICK_CLOEXEC|FSPICK_SYMLINK_NOFOLLOW"
	       "|FSPICK_NO_AUTOMOUNT|FSPICK_EMPTY_PATH",
	       errstr);
# endif

	if (k_fspick(-1, path, 0) < 0)
		printf("fspick(-1, \"%s\", 0) = %s\n",
		       path, errstr);
	else
		printf("fspick(-1, \"%s\", 0) = %s<%s>\n",
		       path, errstr, path);

	if (k_fspick(dfd, empty, 8) < 0)
		printf("fspick(%d<%s>, \"\", %s) = %s\n",
		       dfd, path, "FSPICK_EMPTY_PATH", errstr);
	else
		printf("fspick(%d<%s>, \"\", %s) = %s<%s>\n",
		       dfd, path, "FSPICK_EMPTY_PATH", errstr, path);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fspick")

#endif
