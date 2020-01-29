/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

/*
 * This test is designed to be executed with the following strace options:
 * --secontext[=full] -y
 */

#if defined HAVE_SYS_FANOTIFY_H && defined HAVE_FANOTIFY_MARK && \
	defined __NR_fanotify_mark && defined HAVE_SELINUX_RUNTIME

# include <limits.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/fanotify.h>

/* for getcwd()/opendir() */
# include <limits.h>
# include <sys/types.h>
# include <dirent.h>

# define str_fan_mark_add	"FAN_MARK_ADD"
# define str_fan_modify_ondir	"FAN_MODIFY|FAN_ONDIR"
# define str_at_fdcwd		"AT_FDCWD"

# include "selinux.c"

int
main(void)
{
	int rc;

	rc = fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY | FAN_ONDIR,
			       -100, ".");
	printf("%sfanotify_mark(-1, %s, %s, %s, \".\"%s) = %s\n",
	       SELINUX_MYCONTEXT(),
	       str_fan_mark_add, str_fan_modify_ondir,
	       str_at_fdcwd, SELINUX_FILECONTEXT("."),
	       sprintrc(rc));

	/*
	 * Test with dirfd
	 */

	char cwd[PATH_MAX + 1];
	DIR *dir = NULL;
	if (getcwd(cwd, sizeof (cwd)) == NULL)
		perror_msg_and_fail("getcwd");
	dir = opendir(cwd);
	if (dir == NULL)
		perror_msg_and_fail("opendir");
	int dfd = dirfd(dir);
	if (dfd == -1)
		perror_msg_and_fail("dirfd");

	rc = fanotify_mark(-1, FAN_MARK_ADD, FAN_MODIFY | FAN_ONDIR,
			       dfd, ".");
	printf("%sfanotify_mark(-1, %s, %s, %d<%s>%s, \".\"%s) = %s\n",
	       SELINUX_MYCONTEXT(),
	       str_fan_mark_add, str_fan_modify_ondir,
	       dfd, cwd, SELINUX_FILECONTEXT(cwd),
	       SELINUX_FILECONTEXT("."),
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_FANOTIFY_H && HAVE_FANOTIFY_MARK && "
		    "__NR_fanotify_mark && HAVE_SELINUX_RUNTIME")

#endif
