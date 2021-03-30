/*
 * Check decoding of fchmod syscall.
 *
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
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
# include <stdlib.h>
# include <limits.h>
# include <unistd.h>

# include "selinux.c"

int
main(void)
{
	static const char fname[] = "fchmod_test_file";
	char *my_secontext = SELINUX_MYCONTEXT();

	unlink(fname);
	int fd = open(fname, O_CREAT|O_RDONLY, 0400);
	if (fd == -1)
		perror_msg_and_fail("open");

# ifdef YFLAG
	char *cwd = NULL;
	get_curdir_fd(&cwd);
	char *fname_realpath = xasprintf("%s/%s", cwd, fname);
# endif

	const char *fname_context = SELINUX_FILECONTEXT(fname);
	long rc = syscall(__NR_fchmod, fd, 0600);
# ifdef YFLAG
	printf("%sfchmod(%d<%s>%s, 0600) = %s\n",
# else
	printf("%sfchmod(%d%s, 0600) = %s\n",
# endif
	       my_secontext,
	       fd,
# ifdef YFLAG
	       fname_realpath,
# endif
	       fname_context,
	       sprintrc(rc));

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmod, fd, 051);
# ifdef YFLAG
	printf("%sfchmod(%d<%s (deleted)>%s, 051) = %s\n",
# else
	printf("%sfchmod(%d%s, 051) = %s\n",
# endif
	       my_secontext,
	       fd,
# ifdef YFLAG
	       fname_realpath,
# endif
	       fname_context,
	       sprintrc(rc));

	rc = syscall(__NR_fchmod, fd, 004);
# ifdef YFLAG
	printf("%sfchmod(%d<%s (deleted)>%s, 004) = %s\n",
# else
	printf("%sfchmod(%d%s, 004) = %s\n",
# endif
	       my_secontext,
	       fd,
# ifdef YFLAG
	       fname_realpath,
# endif
	       fname_context,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmod")

#endif
