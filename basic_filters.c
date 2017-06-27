/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2017 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include "filter.h"
#include <regex.h>

typedef unsigned int number_slot_t;
#define BITS_PER_SLOT (sizeof(number_slot_t) * 8)

struct number_set {
	number_slot_t *vec;
	unsigned int nslots;
	bool not;
};

static void
number_setbit(const unsigned int i, number_slot_t *const vec)
{
	vec[i / BITS_PER_SLOT] |= (number_slot_t) 1 << (i % BITS_PER_SLOT);
}

static bool
number_isset(const unsigned int i, const number_slot_t *const vec)
{
	return vec[i / BITS_PER_SLOT] & ((number_slot_t) 1 << (i % BITS_PER_SLOT));
}

static void
reallocate_number_set(struct number_set *const set, const unsigned int new_nslots)
{
	if (new_nslots <= set->nslots)
		return;
	set->vec = xreallocarray(set->vec, new_nslots, sizeof(*set->vec));
	memset(set->vec + set->nslots, 0,
	       sizeof(*set->vec) * (new_nslots - set->nslots));
	set->nslots = new_nslots;
}

void
add_number_to_set(const unsigned int number, struct number_set *const set)
{
	reallocate_number_set(set, number / BITS_PER_SLOT + 1);
	number_setbit(number, set->vec);
}

bool
is_number_in_set(const unsigned int number, const struct number_set *const set)
{
	return ((number / BITS_PER_SLOT < set->nslots)
		&& number_isset(number, set->vec)) ^ set->not;
}

static bool
qualify_syscall_number(const char *s, struct number_set *set)
{
	int n = string_to_uint(s);
	if (n < 0)
		return false;

	unsigned int p;
	bool done = false;

	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		if ((unsigned) n >= nsyscall_vec[p]) {
			continue;
		}
		add_number_to_set(n, &set[p]);
		done = true;
	}

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

	unsigned int p;
	bool found = false;
	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		unsigned int i;

		for (i = 0; i < nsyscall_vec[p]; ++i) {
			if (!sysent_vec[p][i].sys_name)
				continue;
			rc = regexec(&preg, sysent_vec[p][i].sys_name,
				     0, NULL, 0);
			if (rc == REG_NOMATCH)
				continue;
			else if (rc)
				regerror_msg_and_die(rc, &preg, "regexec", s);
			add_number_to_set(i, &set[p]);
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
		{ "desc",	TRACE_DESC	},
		{ "file",	TRACE_FILE	},
		{ "memory",	TRACE_MEMORY	},
		{ "process",	TRACE_PROCESS	},
		{ "signal",	TRACE_SIGNAL	},
		{ "ipc",	TRACE_IPC	},
		{ "network",	TRACE_NETWORK	},
		{ "%desc",	TRACE_DESC	},
		{ "%file",	TRACE_FILE	},
		{ "%memory",	TRACE_MEMORY	},
		{ "%process",	TRACE_PROCESS	},
		{ "%signal",	TRACE_SIGNAL	},
		{ "%ipc",	TRACE_IPC	},
		{ "%network",	TRACE_NETWORK	},
		{ "%stat",	TRACE_STAT	},
		{ "%lstat",	TRACE_LSTAT	},
		{ "%fstat",	TRACE_FSTAT	},
		{ "%%stat",	TRACE_STAT_LIKE	},
		{ "%statfs",	TRACE_STATFS	},
		{ "%fstatfs",	TRACE_FSTATFS	},
		{ "%%statfs",	TRACE_STATFS_LIKE	},
	};

	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(syscall_class); ++i) {
		if (strcmp(s, syscall_class[i].name) == 0) {
			return syscall_class[i].value;
		}
	}

	return 0;
}

static bool
qualify_syscall_class(const char *s, struct number_set *set)
{
	const unsigned int n = lookup_class(s);
	if (!n)
		return false;

	unsigned int p;
	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		unsigned int i;

		for (i = 0; i < nsyscall_vec[p]; ++i) {
			if (!sysent_vec[p][i].sys_name
			    || (sysent_vec[p][i].sys_flags & n) != n) {
				continue;
			}
			add_number_to_set(i, &set[p]);
		}
	}

	return true;
}

static bool
qualify_syscall_name(const char *s, struct number_set *set)
{
	unsigned int p;
	bool found = false;

	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		unsigned int i;

		for (i = 0; i < nsyscall_vec[p]; ++i) {
			if (!sysent_vec[p][i].sys_name
			    || strcmp(s, sysent_vec[p][i].sys_name)) {
				continue;
			}
			add_number_to_set(i, &set[p]);
			found = true;
		}
	}

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
qualify_syscall_tokens(const char *const str, struct number_set *const set,
		       const char *const name)
{
	/* Clear all sets. */
	unsigned int p;
	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		if (set[p].nslots)
			memset(set[p].vec, 0,
			       sizeof(*set[p].vec) * set[p].nslots);
		set[p].not = false;
	}

	/*
	 * Each leading ! character means inversion
	 * of the remaining specification.
	 */
	const char *s = str;
handle_inversion:
	while (*s == '!') {
		for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
			set[p].not = !set[p].not;
		}
		++s;
	}

	if (strcmp(s, "none") == 0) {
		/*
		 * No syscall numbers are added to sets.
		 * Subsequent is_number_in_set invocations
		 * will return set[p]->not.
		 */
		return;
	} else if (strcmp(s, "all") == 0) {
		s = "!none";
		goto handle_inversion;
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
	const char *token;
	bool done = false;

	for (token = strtok_r(copy, ",", &saveptr); token;
	     token = strtok_r(NULL, ",", &saveptr)) {
		done = qualify_syscall(token, set);
		if (!done) {
			error_msg_and_die("invalid %s '%s'", name, token);
		}
	}

	free(copy);

	if (!done) {
		error_msg_and_die("invalid %s '%s'", name, str);
	}
}

/*
 * Add numbers to SET according to STR specification.
 */
void
qualify_tokens(const char *const str, struct number_set *const set,
	       string_to_uint_func func, const char *const name)
{
	/* Clear the set. */
	if (set->nslots)
		memset(set->vec, 0, sizeof(*set->vec) * set->nslots);
	set->not = false;

	/*
	 * Each leading ! character means inversion
	 * of the remaining specification.
	 */
	const char *s = str;
handle_inversion:
	while (*s == '!') {
		set->not = !set->not;
		++s;
	}

	if (strcmp(s, "none") == 0) {
		/*
		 * No numbers are added to the set.
		 * Subsequent is_number_in_set invocations will return set->not.
		 */
		return;
	} else if (strcmp(s, "all") == 0) {
		s = "!none";
		goto handle_inversion;
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
	const char *token;
	int number = -1;

	for (token = strtok_r(copy, ",", &saveptr); token;
	     token = strtok_r(NULL, ",", &saveptr)) {
		number = func(token);
		if (number < 0) {
			error_msg_and_die("invalid %s '%s'", name, token);
		}

		add_number_to_set(number, set);
	}

	free(copy);

	if (number < 0) {
		error_msg_and_die("invalid %s '%s'", name, str);
	}
}
