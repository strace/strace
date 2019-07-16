/*
 * Check decoding of inotify_init1 syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#if defined(__NR_inotify_init1)

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# ifdef O_CLOEXEC
#  define cloexec_flag O_CLOEXEC
# else
#  define cloexec_flag 0
# endif
# define all_flags (O_NONBLOCK | cloexec_flag)

# ifdef PRINT_PATHS
#  define RC_FMT "%ld<%s>"
# else
#  define RC_FMT "%s"
# endif

int
main(void)
{
# ifdef PRINT_PATHS
	skip_if_unavailable("/proc/self/fd/");
# endif

	static const kernel_ulong_t bogus_flags1 =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL | O_NONBLOCK;
	static const kernel_ulong_t bogus_flags2 =
		(kernel_ulong_t) 0x55555550ff96b77bULL & ~all_flags;

	long rc;

	rc = syscall(__NR_inotify_init1, bogus_flags1);
	printf("inotify_init1(IN_NONBLOCK|%s%#x) = %s\n",
	       bogus_flags1 & cloexec_flag  ? "IN_CLOEXEC|" : "",
	       (unsigned int) (bogus_flags1 & ~all_flags),
	       sprintrc(rc));

	rc = syscall(__NR_inotify_init1, bogus_flags2);
	printf("inotify_init1(%#x /* IN_??? */) = %s\n",
	       (unsigned int) bogus_flags2, sprintrc(rc));

	rc = syscall(__NR_inotify_init1, all_flags);

# ifdef PRINT_PATHS
	if (rc < 0)
		perror_msg_and_skip("inotify_init(%#x)", all_flags);

	/*
	 * Kernels that do not have v2.6.33-rc1~34^2~7 do not have
	 * "anon_inode:" prefix.  Let's assume that it can be either "inotify"
	 * or "anon_inode:inotify" for now, as any change there may be
	 * of interest.
	 */
	char path[sizeof("/proc/self/fd/") + sizeof(rc) * 3];
	char buf[2] = "";
	const char *inotify_path;
	ssize_t ret;

	ret = snprintf(path, sizeof(path), "/proc/self/fd/%ld", rc);
	if ((ret < 0) || ((size_t) ret >= sizeof(path)))
		perror_msg_and_fail("snprintf(path)");

	ret = readlink(path, buf, sizeof(buf));
	if (ret < 0)
		perror_msg_and_fail("readlink");

	switch (buf[0]) {
	case 'a':
		inotify_path = "anon_inode:inotify";
		break;
	case 'i':
		inotify_path = "inotify";
		break;
	default:
		error_msg_and_fail("Unexpected first char '%c' of inotify fd "
				   "link path", buf[0]);
	}
# endif

	printf("inotify_init1(IN_NONBLOCK%s) = " RC_FMT "\n",
	       all_flags & cloexec_flag ? "|IN_CLOEXEC" : "",
# ifdef PRINT_PATHS
	       rc, inotify_path
# else
	       sprintrc(rc)
# endif
	       );

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_inotify_init1");

#endif
