/*
 * Check decoding of sync_file_range syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include "scno.h"

#if defined HAVE_SYNC_FILE_RANGE && defined __NR_sync_file_range

# include <stdio.h>

int
main(void)
{
	const int fd = -1;
	const uint64_t offset = (uint64_t) 0xdeadbeefbadc0dedULL;
	const uint64_t nbytes = (uint64_t) 0xfacefeedcafef00dULL;
	const unsigned int flags = -1;

	int rc = sync_file_range(fd, offset, nbytes, flags);
	printf("%s(%d, %lld, %lld, SYNC_FILE_RANGE_WAIT_BEFORE"
	       "|SYNC_FILE_RANGE_WRITE|SYNC_FILE_RANGE_WAIT_AFTER"
	       "|0xfffffff8) = %s\n",
	       "sync_file_range", fd,
	       (long long) offset,
	       (long long) nbytes,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYNC_FILE_RANGE && __NR_sync_file_range")

#endif
