/*
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <selinux/selinux.h>

#include "selinux.h"
#include "xstring.h"

bool selinux_context = false;
bool selinux_context_full = false;

static int
getcontext(int rc, char **secontext, char **result)
{
	if (rc < 0)
		return rc;

	*result = NULL;
	if (!selinux_context_full) {
		char *saveptr = NULL;
		char *secontext_copy = xstrdup(*secontext);
		const char *token;
		unsigned int i;

		/*
		 * We only want to keep the type (3rd field, ':' separator).
		 */
		for (token = strtok_r(secontext_copy, ":", &saveptr), i = 0;
		     token; token = strtok_r(NULL, ":", &saveptr), i++) {
			if (i == 2) {
				*result = xstrdup(token);
				break;
			}
		}
		free(secontext_copy);
	}

	if (*result == NULL)
		*result = xstrdup(*secontext);
	freecon(*secontext);
	return 0;
}
/*
 * Retrieves the SELinux context of the given PID (extracted from the tcb).
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
int
selinux_getpidcon(struct tcb *tcp, char **result)
{
	if (!selinux_context)
		return -1;

	int proc_pid = 0;
	translate_pid(NULL, tcp->pid, PT_TID, &proc_pid);
	if (!proc_pid)
		return -1;

	char *secontext;
	return getcontext(getpidcon(proc_pid, &secontext), &secontext, result);
}

/*
 * Retrieves the SELinux context of the given pid and descriptor.
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
int
selinux_getfdcon(pid_t pid, int fd, char **result)
{
	if (!selinux_context || pid <= 0 || fd < 0)
		return -1;

	int proc_pid = 0;
	translate_pid(NULL, pid, PT_TID, &proc_pid);
	if (!proc_pid)
		return -1;

	char linkpath[sizeof("/proc/%u/fd/%u") + 2 * sizeof(int)*3];
	xsprintf(linkpath, "/proc/%u/fd/%u", proc_pid, fd);

	char *secontext;
	return getcontext(getfilecon(linkpath, &secontext), &secontext, result);
}

/*
 * Retrieves the SELinux context of the given path.
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
int
selinux_getfilecon(struct tcb *tcp, const char *path, char **result)
{
	if (!selinux_context)
		return -1;

	int proc_pid = 0;
	translate_pid(NULL, tcp->pid, PT_TID, &proc_pid);
	if (!proc_pid)
		return -1;

	int ret = -1;
	char fname[PATH_MAX];

	if (path[0] == '/')
		ret = snprintf(fname, sizeof(fname), "/proc/%u/root%s",
			       proc_pid, path);
	else if (tcp->last_dirfd == AT_FDCWD)
		ret = snprintf(fname, sizeof(fname), "/proc/%u/cwd/%s",
			       proc_pid, path);
	else if (tcp->last_dirfd >= 0 )
		ret = snprintf(fname, sizeof(fname), "/proc/%u/fd/%u/%s",
			       proc_pid, tcp->last_dirfd, path);

	if ((unsigned int) ret >= sizeof(fname))
		return -1;

	char *secontext;
	return getcontext(getfilecon(fname, &secontext), &secontext, result);
}
