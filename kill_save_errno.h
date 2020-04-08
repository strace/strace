/*
 * Copyright (c) 2012-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KILL_SAVE_ERRNO_H
# define STRACE_KILL_SAVE_ERRNO_H

# include <sys/types.h>
# include <signal.h>
# include <errno.h>

static inline int
kill_save_errno(pid_t pid, int sig)
{
	int saved_errno = errno;
	int rc = kill(pid, sig);
	errno = saved_errno;
	return rc;
}

#endif /* !STRACE_KILL_SAVE_ERRNO_H */
