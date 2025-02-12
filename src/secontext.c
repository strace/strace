/*
 * Copyright (c) 2020-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <stdlib.h>
#include <fcntl.h>
#include <libgen.h>
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

static char unknown_expected_context[] = "??";

/**
 * @param secontext Pointer to security context string.
 * @param result    Stores pointer to the beginning of the part to print.
 * @return          Number of characters of the string to be printed.
 */
static size_t
parse_secontext(char *secontext, char **result)
{
	char *end_pos = NULL;

	if (!is_number_in_set(SECONTEXT_FULL, secontext_set)) {
		/* We're looking for the type which is the third field */
		enum { SECONTEXT_TYPE = 2 };
		char *start_pos = secontext;

		for (unsigned int i = 0; i <= SECONTEXT_TYPE; i++) {
			end_pos = strchr(start_pos, ':');

			if (i == SECONTEXT_TYPE) {
				secontext = start_pos;
				break;
			}

			if (!end_pos)
				break;

			start_pos = end_pos + 1;
		}
	}

	size_t len = end_pos ? (size_t) (end_pos - secontext)
			     : strlen(secontext);

	/* Strip terminating \n as these tend to be present sometimes */
	while (len && secontext[len - 1] == '\n')
		len--;

	return *result = secontext, len;
}

static int
get_expected_filecontext(const char *path, char **secontext, int mode)
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

	return selabel_lookup(hdl, secontext, path, mode);
}

/*
 * Retrieves the SELinux context of the given PID (extracted from the tcb).
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
static int
selinux_getpidcon(struct tcb *tcp, char **secontext)
{
	if (number_set_array_is_empty(secontext_set, 0))
		return -1;

	int proc_pid = get_proc_pid(tcp->pid);
	if (!proc_pid)
		return -1;

	return getpidcon(proc_pid, secontext);
}

/*
 * Retrieves the SELinux context of the given pid and descriptor.
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
static int
selinux_getfdcon(pid_t pid, int fd, char **secontext, char **expected)
{
	if (number_set_array_is_empty(secontext_set, 0) || pid <= 0 || fd < 0)
		return -1;

	int proc_pid = get_proc_pid(pid);
	if (!proc_pid)
		return -1;

	int rc;
	char buf[PATH_MAX + 1];

	rc = snprintf(buf, sizeof(buf), "/proc/%u/fd/%u", proc_pid, fd);
	if ((unsigned int) rc >= sizeof(buf))
		return -1;

	rc = getfilecon(buf, secontext);
	if (rc < 0 || !is_number_in_set(SECONTEXT_MISMATCH, secontext_set))
		return rc;

	/*
	 * We need to resolve the path, because selabel_lookup() doesn't
	 * resolve anything.
	 */
	ssize_t n = get_proc_pid_fd_path(proc_pid, fd, buf, sizeof(buf), NULL);
	if ((size_t) n >= (sizeof(buf) - 1))
		return 0;

	/*
	 * We retrieve stat() here since the path the procfs link resolves into
	 * may be reused by a different file with different context.
	 */
	strace_stat_t st;
	if (stat_file(buf, &st) < 0)
		return 0;

	if ((get_expected_filecontext(buf, expected, st.st_mode) < 0) && (errno != ENOENT))
		*expected = unknown_expected_context;

	return 0;
}

/*
 * Retrieves the SELinux context of the given path.
 * Memory must be freed.
 * Returns 0 on success, -1 on failure.
 */
static int
selinux_getfilecon(struct tcb *tcp, const char *path, char **secontext,
		   char **expected)
{
	if (number_set_array_is_empty(secontext_set, 0))
		return -1;

	int proc_pid = get_proc_pid(tcp->pid);
	if (!proc_pid)
		return -1;

	/*
	 * The path may contain .../proc/self/... or .../proc/self<eos>.
	 * We need to replace 'self' by the target PID, or else we would
	 * resolve the path with 'self' being strace itself.
	 * In some corner cases, it's impossible to guess so keep it as is if
	 * we don't find any occurrence of /proc/self.
	 * Finally there may be breakage if the pathname contains /proc/self
	 * but has nothing to do with /proc.
	 */

	int rc;
	char fname[PATH_MAX];
	char buf[PATH_MAX + 1];
	size_t l = strlcpy(fname, path, sizeof(fname));
	if (l >= sizeof(fname) - 1)
		return -1;
	int proc_self_substituted = 0;

	for (;;) {
		char *s = strstr(fname, "/proc/self/");
		if (s == NULL) {
			/* Corner case: check if the path ends with /proc/self */
			s = fname + strlen(fname) - sizeof("/proc/self") + 1;
			if (strcmp("/proc/self", s))
				s = NULL;
		}
		if (s == NULL)
			break;
		*s = '\0';
		rc = snprintf(buf, sizeof(buf), "%s/proc/%u%s",
			fname, proc_pid, s + sizeof("/proc/self") - 1);
		if ((unsigned int) rc >= sizeof(buf))
			return -1;
		strcpy(fname, buf);
		proc_self_substituted = 1;
	}

	strcpy(buf, fname);

retry:
	if (buf[0] == '/')
		rc = snprintf(fname, sizeof(fname), "/proc/%u/root%s",
			       proc_pid, buf);
	else if (tcp->last_dirfd == AT_FDCWD)
		rc = snprintf(fname, sizeof(fname), "/proc/%u/cwd/%s",
			       proc_pid, buf);
	else if (tcp->last_dirfd >= 0 )
		rc = snprintf(fname, sizeof(fname), "/proc/%u/fd/%u/%s",
			       proc_pid, tcp->last_dirfd, buf);
	else
		return -1;

	if ((unsigned int) rc >= sizeof(fname))
		return -1;

	rc = lgetfilecon(fname, secontext);
	if ((rc < 0) && (errno == ENOENT) && proc_self_substituted) {
		proc_self_substituted = 0;
		strcpy(buf, path);
		goto retry;
	}
	if (rc < 0 || !is_number_in_set(SECONTEXT_MISMATCH, secontext_set))
		return rc;

	/*
	 * We need to fully resolve the path, because selabel_lookup() doesn't
	 * resolve anything.  Using realpath() is the only solution here to make
	 * sure the path is canonicalized.
	 * There are two cases:
	 * - if the inode is a symlink, resolve only the dirname, because we we
	 *   want to get the expected context of the symlink itself, not the
	 *   target context
	 * - otherwise resolve everything because we will get same result
	 */

	strace_stat_t st;
	if (lstat_file(fname, &st) < 0)
		return 0;

	if (S_ISLNK(st.st_mode)) {
		strcpy(buf, fname);
		char *d = dirname(buf);

		char *resolved = realpath(d, NULL);
		if (!resolved)
			return 0;

		/* This breaks fname but we don't care anymore */
		char *b = basename(fname);

		rc = snprintf(buf, sizeof(buf), "%s/%s", resolved, b);
		free(resolved);
		if ((unsigned int) rc >= sizeof(buf))
			return 0;
	} else {
		char *resolved = realpath(fname, NULL);
		if (!resolved)
			return 0;
		strcpy(buf, resolved);
		free(resolved);
	}

	if ((get_expected_filecontext(buf, expected, st.st_mode) < 0) && (errno != ENOENT))
		*expected = unknown_expected_context;

	return 0;
}

static void
print_context(char *secontext, char *expected)
{
	if (!secontext)
		return;

	unsigned int style = QUOTE_OMIT_LEADING_TRAILING_QUOTES
			     | QUOTE_OVERWRITE_HEXSTR |
			     (xflag == HEXSTR_NONE
			      ? QUOTE_HEXSTR_NONE
			      : QUOTE_HEXSTR_NON_ASCII_CHARS);

	char *ctx_str;
	ssize_t ctx_len = parse_secontext(secontext, &ctx_str);

	print_quoted_string_ex(ctx_str, ctx_len, style, "[]!");

	if (!expected)
		goto freecon_secontext;

	char *exp_str;
	ssize_t exp_len = parse_secontext(expected, &exp_str);

	if (ctx_len != exp_len || strncmp(ctx_str, exp_str, ctx_len)) {
		tprints_string("!!");
		print_quoted_string_ex(exp_str, exp_len, style, "[]!");
	}

	if (expected != unknown_expected_context)
		freecon(expected);
freecon_secontext:
	freecon(secontext);
}

void
selinux_printfdcon(pid_t pid, int fd)
{
	char *ctx = NULL;
	char *exp = NULL;

	if (selinux_getfdcon(pid, fd, &ctx, &exp) < 0)
		return;

	tprint_space();
	tprint_attribute_begin();
	print_context(ctx, exp);
	tprint_attribute_end();
}

void
selinux_printfilecon(struct tcb *tcp, const char *path)
{
	char *ctx = NULL;
	char *exp = NULL;

	if (selinux_getfilecon(tcp, path, &ctx, &exp) < 0)
		return;

	tprint_space();
	tprint_attribute_begin();
	print_context(ctx, exp);
	tprint_attribute_end();
}

void
selinux_printpidcon(struct tcb *tcp)
{
	char *ctx = NULL;

	if (selinux_getpidcon(tcp, &ctx) < 0)
		return;

	tprint_attribute_begin();
	print_context(ctx, NULL);
	tprint_attribute_end();
	tprint_space();
}
