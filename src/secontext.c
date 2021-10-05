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
#include <sys/stat.h>
#include <unistd.h>
#include <selinux/selinux.h>
#include <selinux/label.h>

#include "largefile_wrappers.h"
#include "number_set.h"
#include "secontext.h"
#include "xmalloc.h"
#include "xstring.h"

static int
getcontext(int rc, char **secontext, char **result)
{
	if (rc < 0)
		return rc;

	*result = NULL;
	if (!is_number_in_set(SECONTEXT_FULL, secontext_set)) {
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

	if (*result == NULL) {
		/*
		 * On the CI at least, the context may have a trailing \n,
		 * let's remove it just in case.
		 */
		size_t len = strlen(*secontext);
		for (; len > 0; --len) {
			if ((*secontext)[len - 1] != '\n')
				break;
		}
		*result = xstrndup(*secontext, len);
	}
	freecon(*secontext);
	return 0;
}

static int
get_expected_filecontext(const char *path, char **result)
{
	static struct selabel_handle *hdl;

	if (!hdl) {
		static bool disabled;
		if (disabled)
			return -1;

		hdl = selabel_open(SELABEL_CTX_FILE, NULL, 0);
		if (!hdl) {
			perror_msg("could not open SELinux database, disabling "
				   "context mismatch checking");
			disabled = true;
			return -1;
		}
	}

	strace_stat_t stb;
	if (stat_file(path, &stb) < 0) {
		return -1;
	}

	char *secontext;
	return getcontext(selabel_lookup(hdl, &secontext, path, stb.st_mode),
			  &secontext, result);
}

/*
 * Retrieves the SELinux context of the given PID (extracted from the tcb).
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
int
selinux_getpidcon(struct tcb *tcp, char **result)
{
	if (number_set_array_is_empty(secontext_set, 0))
		return -1;

	int proc_pid = get_proc_pid(tcp->pid);
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
	if (number_set_array_is_empty(secontext_set, 0) || pid <= 0 || fd < 0)
		return -1;

	int proc_pid = get_proc_pid(pid);
	if (!proc_pid)
		return -1;

	char linkpath[sizeof("/proc/%u/fd/%u") + 2 * sizeof(int)*3];
	xsprintf(linkpath, "/proc/%u/fd/%u", proc_pid, fd);

	char *secontext;
	int rc = getcontext(getfilecon(linkpath, &secontext), &secontext, result);
	if (rc < 0 || !is_number_in_set(SECONTEXT_MISMATCH, secontext_set))
		return rc;

	/*
	 * We need to resolve the path, because selabel_lookup() doesn't
	 * resolve anything.  Using readlink() is sufficient here.
	 */

	char buf[PATH_MAX];
	ssize_t n = readlink(linkpath, buf, sizeof(buf));
	if ((size_t) n >= sizeof(buf))
		return 0;
	buf[n] = '\0';

	char *expected;
	if (get_expected_filecontext(buf, &expected) < 0)
		return 0;
	if (strcmp(expected, *result) == 0) {
		free(expected);
		return 0;
	}
	char *final_result = xasprintf("%s!!%s", *result, expected);
	free(expected);
	free(*result);
	*result = final_result;
	return 0;
}

/*
 * Retrieves the SELinux context of the given path.
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
int
selinux_getfilecon(struct tcb *tcp, const char *path, char **result)
{
	if (number_set_array_is_empty(secontext_set, 0))
		return -1;

	int proc_pid = get_proc_pid(tcp->pid);
	if (!proc_pid)
		return -1;

	int rc = -1;
	char fname[PATH_MAX];

	if (path[0] == '/')
		rc = snprintf(fname, sizeof(fname), "/proc/%u/root%s",
			       proc_pid, path);
	else if (tcp->last_dirfd == AT_FDCWD)
		rc = snprintf(fname, sizeof(fname), "/proc/%u/cwd/%s",
			       proc_pid, path);
	else if (tcp->last_dirfd >= 0 )
		rc = snprintf(fname, sizeof(fname), "/proc/%u/fd/%u/%s",
			       proc_pid, tcp->last_dirfd, path);

	if ((unsigned int) rc >= sizeof(fname))
		return -1;

	char *secontext;
	rc = getcontext(getfilecon(fname, &secontext), &secontext, result);
	if (rc < 0 || !is_number_in_set(SECONTEXT_MISMATCH, secontext_set))
		return rc;

	/*
	 * We need to fully resolve the path, because selabel_lookup() doesn't
	 * resolve anything.  Using realpath() is the only solution here to make
	 * sure the path is canonicalized.
	 */

	char *resolved = realpath(fname, NULL);
	if (!resolved)
		return 0;

	char *expected;
	rc = get_expected_filecontext(resolved, &expected);
	free(resolved);
	if (rc < 0)
		return 0;
	if (strcmp(expected, *result) == 0) {
		free(expected);
		return 0;
	}
	char *final_result = xasprintf("%s!!%s", *result, expected);
	free(expected);
	free(*result);
	*result = final_result;
	return 0;
}
