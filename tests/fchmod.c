/*
 * Check decoding of fchmod syscall.
 *
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fchmod

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "fchmod_sample_file";
	(void) unlink(sample);
	int fd = open(sample, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");

# ifdef YFLAG
	char *sample_realpath = get_fd_path(fd);
# endif

	long rc = syscall(__NR_fchmod, fd, 0600);
# ifdef YFLAG
	printf("fchmod(%d<%s>, 0600) = %s\n",
# else
	printf("fchmod(%d, 0600) = %s\n",
# endif
	       fd,
# ifdef YFLAG
	       sample_realpath,
# endif
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmod, fd, 051);
# ifdef YFLAG
	printf("fchmod(%d<%s (deleted)>, 051) = %s\n",
# else
	printf("fchmod(%d, 051) = %s\n",
# endif
	       fd,
# ifdef YFLAG
	       sample_realpath,
# endif
	       sprintrc(rc));

	rc = syscall(__NR_fchmod, fd, 004);
# ifdef YFLAG
	printf("fchmod(%d<%s (deleted)>, 004) = %s\n",
# else
	printf("fchmod(%d, 004) = %s\n",
# endif
	       fd,
# ifdef YFLAG
	       sample_realpath,
# endif
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmod")

#endif
