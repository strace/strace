/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <regex.h>

#include "filter.h"
#include "number_set.h"
#include "xstring.h"


/**
 * Checks whether a @-separated personality specification suffix is present.
 * Personality suffix is a one of strings stored in personality_designators
 * array.
 *
 * @param[in]  s Specification string to check.
 * @param[out] p Where to store personality number if it is found.
 * @return       If personality is found, the provided string is copied without
 *               suffix and returned as a result (callee should de-alllocate it
 *               with free() after use), and personality number is written to p.
 *               Otherwise, NULL is returned and p is untouched.
 */
static char *
qualify_syscall_separate_personality(const char *s, unsigned int *p)
{
	char *pos = strchr(s, '@');

	if (!pos)
		return NULL;

	for (unsigned int i = 0; i < SUPPORTED_PERSONALITIES; i++) {
		if (!strcmp(pos + 1, personality_designators[i])) {
			*p = i;
			return xstrndup(s, pos - s);
		}
	}

	error_msg_and_help("incorrect personality designator '%s'"
			   " in qualification '%s'", pos + 1, s);
}

static bool
qualify_syscall_number_personality(int n, unsigned int p,
				   struct number_set *set)
{
	if ((unsigned int) n >= nsyscall_vec[p])
		return false;

	add_number_to_set_array(n, set, p);

	return true;
}

static bool
qualify_syscall_number(const char *s, struct number_set *set)
{
	unsigned int p;
	char *num_str = qualify_syscall_separate_personality(s, &p);
	int n;

	if (num_str) {
		n = string_to_uint(num_str);
		free(num_str);

		if (n < 0)
			return false;

		return qualify_syscall_number_personality(n, p, set);
	}

	n = string_to_uint(s);
	if (n < 0)
		return false;

	bool done = false;

	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p)
		done |= qualify_syscall_number_personality(n, p, set);

	return done;
}

static void
regerror_msg_and_die(int errcode, const regex_t *preg,
		     const char *str, const char *pattern)
{
	char buf[512];

	regerror(errcode, preg, buf, sizeof(buf));
	error_msg_and_die("%s: %s: %s", str, pattern, buf);
}

static bool
qualify_syscall_regex(const char *s, struct number_set *set)
{
	regex_t preg;
	int rc;

	if ((rc = regcomp(&preg, s, REG_EXTENDED | REG_NOSUB)) != 0)
		regerror_msg_and_die(rc, &preg, "regcomp", s);

	bool found = false;

	for (unsigned int p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
			if (!sysent_vec[p][i].sys_name)
				continue;

			rc = regexec(&preg, sysent_vec[p][i].sys_name,
				     0, NULL, 0);

			if (rc == REG_NOMATCH) {
				char name_buf[128];
				char *pos = stpcpy(name_buf,
						   sysent_vec[p][i].sys_name);

				(void) xappendstr(name_buf, pos, "@%s",
						  personality_designators[p]);

				rc = regexec(&preg, name_buf, 0, NULL, 0);
			}

			if (rc == REG_NOMATCH)
				continue;
			else if (rc)
				regerror_msg_and_die(rc, &preg, "regexec", s);

			add_number_to_set_array(i, set, p);
			found = true;
		}
	}

	regfree(&preg);
	return found;
}

static unsigned int
lookup_class(const char *s)
{
	static const struct {
		const char *name;
		unsigned int value;
	} syscall_class[] = {
		{ "%desc",	TRACE_DESC	},
		{ "%file",	TRACE_FILE	},
		{ "%memory",	TRACE_MEMORY	},
		{ "%process",	TRACE_PROCESS	},
		{ "%creds",	TRACE_CREDS	},
		{ "%signal",	TRACE_SIGNAL	},
		{ "%ipc",	TRACE_IPC	},
		{ "%net",	TRACE_NETWORK	},
		{ "%network",	TRACE_NETWORK	},
		{ "%stat",	TRACE_STAT	},
		{ "%lstat",	TRACE_LSTAT	},
		{ "%fstat",	TRACE_FSTAT	},
		{ "%%stat",	TRACE_STAT_LIKE	},
		{ "%statfs",	TRACE_STATFS	},
		{ "%fstatfs",	TRACE_FSTATFS	},
		{ "%%statfs",	TRACE_STATFS_LIKE	},
		{ "%pure",	TRACE_PURE	},
		{ "%clock",	TRACE_CLOCK	},
		/* legacy class names */
		{ "desc",	TRACE_DESC	},
		{ "file",	TRACE_FILE	},
		{ "memory",	TRACE_MEMORY	},
		{ "process",	TRACE_PROCESS	},
		{ "signal",	TRACE_SIGNAL	},
		{ "ipc",	TRACE_IPC	},
		{ "network",	TRACE_NETWORK	},
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(syscall_class); ++i) {
		if (strcmp(s, syscall_class[i].name) == 0)
			return syscall_class[i].value;
	}

	return 0;
}

static bool
qualify_syscall_class(const char *s, struct number_set *set)
{
	const unsigned int n = lookup_class(s);
	if (!n)
		return false;

	for (unsigned int p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
			if (sysent_vec[p][i].sys_name &&
			    (sysent_vec[p][i].sys_flags & n) == n)
				add_number_to_set_array(i, set, p);
		}
	}

	return true;
}

kernel_long_t
scno_by_name(const char *s, unsigned int p, kernel_long_t start)
{
	if (p >= SUPPORTED_PERSONALITIES)
		return -1;

	for (kernel_ulong_t i = start; i < nsyscall_vec[p]; ++i) {
		if (sysent_vec[p][i].sys_name &&
		    strcmp(s, sysent_vec[p][i].sys_name) == 0)
			return i;
	}

	return -1;
}

static bool
qualify_syscall_name_personality(const char *s, unsigned int p,
				 struct number_set *set)
{
	bool found = false;

	for (kernel_long_t scno = 0; (scno = scno_by_name(s, p, scno)) >= 0;
	     ++scno) {
		add_number_to_set_array(scno, set, p);
		found = true;
	}

	return found;
}

static bool
qualify_syscall_name(const char *s, struct number_set *set)
{
	unsigned int p;
	char *name_str = qualify_syscall_separate_personality(s, &p);
	bool found = false;

	if (name_str) {
		found = qualify_syscall_name_personality(name_str, p, set);
		free(name_str);

		return found;
	}

	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p)
		found |= qualify_syscall_name_personality(s, p, set);

	return found;
}

static bool
qualify_syscall(const char *token, struct number_set *set)
{
	bool ignore_fail = false;

	while (*token == '?') {
		token++;
		ignore_fail = true;
	}
	if (*token >= '0' && *token <= '9')
		return qualify_syscall_number(token, set) || ignore_fail;
	if (*token == '/')
		return qualify_syscall_regex(token + 1, set) || ignore_fail;
	return qualify_syscall_class(token, set)
	       || qualify_syscall_name(token, set)
	       || ignore_fail;
}

/*
 * Add syscall numbers to SETs for each supported personality
 * according to STR specification.
 */
void
qualify_syscall_tokens(const char *const str, struct number_set *const set)
{
	/* Clear all sets. */
	clear_number_set_array(set, SUPPORTED_PERSONALITIES);

	/*
	 * Each leading ! character means inversion
	 * of the remaining specification.
	 */
	const char *s = str;
	while (*s == '!') {
		invert_number_set_array(set, SUPPORTED_PERSONALITIES);
		++s;
	}

	if (strcmp(s, "none") == 0) {
		/*
		 * No syscall numbers are added to sets.
		 * Subsequent is_number_in_set* invocations
		 * will return set[p]->not.
		 */
		return;
	} else if (strcmp(s, "all") == 0) {
		/* "all" == "!none" */
		invert_number_set_array(set, SUPPORTED_PERSONALITIES);
		return;
	}

	/*
	 * Split the string into comma separated tokens.
	 * For each token, call qualify_syscall that will take care
	 * if adding appropriate syscall numbers to sets.
	 * The absence of tokens or a negative return code
	 * from qualify_syscall is a fatal error.
	 */
	char *copy = xstrdup(s);
	char *saveptr = NULL;
	bool done = false;

	for (const char *token = strtok_r(copy, ",", &saveptr);
	     token; token = strtok_r(NULL, ",", &saveptr)) {
		done = qualify_syscall(token, set);
		if (!done)
			error_msg_and_die("invalid system call '%s'", token);
	}

	free(copy);

	if (!done)
		error_msg_and_die("invalid system call '%s'", str);
}

/*
 * Add numbers to SET according to STR specification.
 */
void
qualify_tokens(const char *const str, struct number_set *const set,
	       string_to_uint_func func, const char *const name)
{
	/* Clear the set. */
	clear_number_set_array(set, 1);

	/*
	 * Each leading ! character means inversion
	 * of the remaining specification.
	 */
	const char *s = str;
	while (*s == '!') {
		invert_number_set_array(set, 1);
		++s;
	}

	if (strcmp(s, "none") == 0) {
		/*
		 * No numbers are added to the set.
		 * Subsequent is_number_in_set* invocations
		 * will return set->not.
		 */
		return;
	} else if (strcmp(s, "all") == 0) {
		/* "all" == "!none" */
		invert_number_set_array(set, 1);
		return;
	}

	/*
	 * Split the string into comma separated tokens.
	 * For each token, find out the corresponding number
	 * by calling FUNC, and add that number to the set.
	 * The absence of tokens or a negative answer
	 * from FUNC is a fatal error.
	 */
	char *copy = xstrdup(s);
	char *saveptr = NULL;
	int number = -1;

	for (const char *token = strtok_r(copy, ",", &saveptr);
	     token; token = strtok_r(NULL, ",", &saveptr)) {
		number = func(token);
		if (number < 0)
			error_msg_and_die("invalid %s '%s'", name, token);

		add_number_to_set(number, set);
	}

	free(copy);

	if (number < 0)
		error_msg_and_die("invalid %s '%s'", name, str);
}
