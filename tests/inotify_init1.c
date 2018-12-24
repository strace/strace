/*
 * Check decoding of inotify_init1 syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <asm/unistd.h>

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

int
main(void)
{
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
	printf("inotify_init1(IN_NONBLOCK%s) = %s\n",
	       all_flags & cloexec_flag ? "|IN_CLOEXEC" : "", sprintrc(rc));

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_inotify_init1");

#endif
