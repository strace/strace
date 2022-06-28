/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <regex.h>

#include "filter.h"
#include "number_set.h"
#include "xstring.h"

/* Personality designators to be used for specifying personality */
static const char *const personality_designators[] = PERSONALITY_DESIGNATORS;
static_assert(ARRAY_SIZE(personality_designators) == SUPPORTED_PERSONALITIES,
	      "ARRAY_SIZE(personality_designators) != SUPPORTED_PERSONALITIES");

/**
 * Checks whether a @-separated personality specification suffix is present.
 * Personality suffix is a one of strings stored in personality_designators
 * array.
 *
 * @param[in]  s Specification string to check.
 * @param[out] p Where to store personality number if it is found.
 * @return       If personality is found, the provided string is copied without
 *               suffix and returned as a result (callee should de-allocate it
 *               with free() after use), and personality number is written to p.
 *               Otherwise, NULL is returned and p is untouched.
 */
static char *
qualify_syscall_separate_personality(const char *s, unsigned int *p)
{
	const char *pos = strrchr(s, '@');

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
qualify_syscall_number(const char *str, unsigned int p, struct number_set *set)
{
	unsigned int nr = string_to_uint(str);
	unsigned int scno = shuffle_scno_pers(nr, p);
	if (!scno_pers_is_valid(scno, p)) {
		if (ARCH_NEEDS_NON_SHUFFLED_SCNO_CHECK &&
		    scno_pers_is_valid(nr, p))
			scno = nr;
		else
			return false;
	}

	add_number_to_set_array(scno, set, p);
	return true;
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
qualify_syscall_regex(const char *str, unsigned int p, struct number_set *set)
{
	regex_t preg;
	int rc;

	if ((rc = regcomp(&preg, str, REG_EXTENDED | REG_NOSUB)) != 0)
		regerror_msg_and_die(rc, &preg, "regcomp", str);

	bool found = false;

	for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
		if (!sysent_vec[p][i].sys_name)
			continue;

		rc = regexec(&preg, sysent_vec[p][i].sys_name,
			     0, NULL, 0);
		if (rc == REG_NOMATCH)
			continue;
		else if (rc)
			regerror_msg_and_die(rc, &preg, "regexec", str);

		add_number_to_set_array(i, set, p);
		found = true;
	}

	regfree(&preg);
	return found;
}

static bool
qualify_syscall_class(const char *str, unsigned int p, struct number_set *set)
{
	static const struct xlat_data syscall_class[] = {
		{ TRACE_DESC,		"%desc" },
		{ TRACE_FILE,		"%file" },
		{ TRACE_MEMORY,		"%memory" },
		{ TRACE_PROCESS,	"%process" },
		{ TRACE_CREDS,		"%creds" },
		{ TRACE_SIGNAL,		"%signal" },
		{ TRACE_IPC,		"%ipc" },
		{ TRACE_NETWORK,	"%net" },
		{ TRACE_NETWORK,	"%network" },
		{ TRACE_STAT,		"%stat" },
		{ TRACE_LSTAT,		"%lstat" },
		{ TRACE_FSTAT,		"%fstat" },
		{ TRACE_STAT_LIKE,	"%%stat" },
		{ TRACE_STATFS,		"%statfs" },
		{ TRACE_FSTATFS,	"%fstatfs" },
		{ TRACE_STATFS_LIKE,	"%%statfs" },
		{ TRACE_PURE,		"%pure" },
		{ TRACE_CLOCK,		"%clock" },
		/* legacy class names */
		{ 0,			"all" },
		{ TRACE_DESC,		"desc" },
		{ TRACE_FILE,		"file" },
		{ TRACE_MEMORY,		"memory" },
		{ TRACE_PROCESS,	"process" },
		{ TRACE_SIGNAL,		"signal" },
		{ TRACE_IPC,		"ipc" },
		{ TRACE_NETWORK,	"network" },
	};
	const struct xlat_data *class = find_xlat_val_case(syscall_class, str);
	if (!class)
		return false;

	unsigned int n = class->val;

	for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
		if (sysent_vec[p][i].sys_name &&
		    (sysent_vec[p][i].sys_flags & n) == n)
			add_number_to_set_array(i, set, p);
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
qualify_syscall_name(const char *s, unsigned int p, struct number_set *set)
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
qualify_syscall_pers(const char *token, unsigned int p, struct number_set *set)
{
	if (*token >= '0' && *token <= '9')
		return qualify_syscall_number(token, p, set);
	if (*token == '/')
		return qualify_syscall_regex(token + 1, p, set);
	return qualify_syscall_class(token, p, set)
	       || qualify_syscall_name(token, p, set);
}

static bool
qualify_syscall(const char *token, struct number_set *set)
{
	bool rc = false;
	while (*token == '?') {
		++token;
		rc = true;
	}

	unsigned int p;
	char *str = qualify_syscall_separate_personality(token, &p);
	if (str) {
		rc |= qualify_syscall_pers(str, p, set);
		free(str);
	} else {
		for (p = 0; p < SUPPORTED_PERSONALITIES; ++p)
			rc |= qualify_syscall_pers(token, p, set);
	}

	return rc;
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
