/*
 * Check decoding of fchmod syscall.
 *
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fchmod

# include <fcntl.h>
# include <sys/stat.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	int fd = create_tmpfile(O_RDWR);

	long rc = syscall(__NR_fchmod, fd, 0600);
	printf("fchmod(%d, 0600) = %s\n", fd, sprintrc(rc));

	close(fd);

	rc = syscall(__NR_fchmod, fd, 051);
	printf("fchmod(%d, 051) = %s\n", fd, sprintrc(rc));

	rc = syscall(__NR_fchmod, fd, 004);
	printf("fchmod(%d, 004) = %s\n", fd, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmod")

#endif
