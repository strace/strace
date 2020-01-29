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
 * Retrieves the SELinux context of the given path.
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
int
selinux_getfilecon(struct tcb *tcp, const char *path, char **result)
{
	if (!selinux_context)
		return -1;

	/*
	 * Current limitation: we cannot query the path if we are in different
	 * mount namespaces.
	 */
	if (get_mnt_ns(tcp) != get_our_mnt_ns())
		return -1;

	char *secontext;

	if (path[0] == '/')
		return getcontext(getfilecon(path, &secontext), &secontext, result);

	char resolved[PATH_MAX + 1];

	/*
	 * If we have a relative pathname and 'last_dirfd' == AT_FDCWD, we need
	 * to prepend by the CWD.
	 */
	if (tcp->last_dirfd == AT_FDCWD) {
		int proc_pid = 0;
		translate_pid(NULL, tcp->pid, PT_TID, &proc_pid);
		if (!proc_pid)
			return -1;

		char linkpath[sizeof("/proc/%u/cwd") + sizeof(int)*3];
		xsprintf(linkpath, "/proc/%u/cwd", proc_pid);

		ssize_t n = readlink(linkpath, resolved, sizeof(resolved) - 1);
		/*
		 * NB: if buffer is too small, readlink doesn't fail,
		 * it returns truncated result.
		 */
		if (n == sizeof(resolved) - 1)
			return -1;
		resolved[n] = '\0';
	} else {
		if (getfdpath_pid(tcp->pid, tcp->last_dirfd,
				  resolved, sizeof(resolved)) < 0)
			return -1;
	}

	if (resolved[0] != '/')
		return -1;

	char pathtoresolve[2 * PATH_MAX + 2];
	xsprintf(pathtoresolve, "%s/%s", resolved, path);

	return getcontext(getfilecon(pathtoresolve, &secontext), &secontext, result);
}
