/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include "nsig.h"
#include <regex.h>

typedef unsigned int number_slot_t;
#define BITS_PER_SLOT (sizeof(number_slot_t) * 8)

struct number_set {
	number_slot_t *vec;
	unsigned int nslots;
	bool not;
};

struct number_set read_set;
struct number_set write_set;
struct number_set signal_set;

static struct number_set abbrev_set[SUPPORTED_PERSONALITIES];
static struct number_set inject_set[SUPPORTED_PERSONALITIES];
static struct number_set raw_set[SUPPORTED_PERSONALITIES];
static struct number_set trace_set[SUPPORTED_PERSONALITIES];
static struct number_set verbose_set[SUPPORTED_PERSONALITIES];

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

static void
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

typedef int (*string_to_uint_func)(const char *);

/*
 * Add numbers to SET according to STR specification.
 */
static void
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

static int
sigstr_to_uint(const char *s)
{
	int i;

	if (*s >= '0' && *s <= '9')
		return string_to_uint_upto(s, 255);

	if (strncasecmp(s, "SIG", 3) == 0)
		s += 3;

	for (i = 0; i <= 255; ++i) {
		const char *name = signame(i);

		if (strncasecmp(name, "SIG", 3) != 0)
			continue;

		name += 3;

		if (strcasecmp(name, s) != 0)
			continue;

		return i;
	}

	return -1;
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
static void
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
 * Returns NULL if STR does not start with PREFIX,
 * or a pointer to the first char in STR after PREFIX.
 */
static const char *
strip_prefix(const char *prefix, const char *str)
{
	size_t len = strlen(prefix);

	return strncmp(prefix, str, len) ? NULL : str + len;
}

static int
find_errno_by_name(const char *name)
{
	unsigned int i;

	for (i = 1; i < nerrnos; ++i) {
		if (errnoent[i] && (strcasecmp(name, errnoent[i]) == 0))
			return i;
	}

	return -1;
}

static bool
parse_inject_token(const char *const token, struct inject_opts *const fopts,
		   const bool fault_tokens_only)
{
	const char *val;
	int intval;

	if ((val = strip_prefix("when=", token))) {
		/*
		 * 	== 1+1
		 * F	== F+0
		 * F+	== F+1
		 * F+S
		 */
		char *end;
		intval = string_to_uint_ex(val, &end, 0xffff, "+");
		if (intval < 1)
			return false;

		fopts->first = intval;

		if (*end) {
			val = end + 1;
			if (*val) {
				/* F+S */
				intval = string_to_uint_upto(val, 0xffff);
				if (intval < 1)
					return false;
				fopts->step = intval;
			} else {
				/* F+ == F+1 */
				fopts->step = 1;
			}
		} else {
			/* F == F+0 */
			fopts->step = 0;
		}
	} else if ((val = strip_prefix("error=", token))) {
		if (fopts->rval != INJECT_OPTS_RVAL_DEFAULT)
			return false;
		intval = string_to_uint_upto(val, MAX_ERRNO_VALUE);
		if (intval < 0)
			intval = find_errno_by_name(val);
		if (intval < 1)
			return false;
		fopts->rval = -intval;
	} else if (!fault_tokens_only && (val = strip_prefix("retval=", token))) {
		if (fopts->rval != INJECT_OPTS_RVAL_DEFAULT)
			return false;
		intval = string_to_uint(val);
		if (intval < 0)
			return false;
		fopts->rval = intval;
	} else if (!fault_tokens_only && (val = strip_prefix("signal=", token))) {
		intval = sigstr_to_uint(val);
		if (intval < 1 || intval > NSIG_BYTES * 8)
			return false;
		fopts->signo = intval;
	} else {
		return false;
	}

	return true;
}

static char *
parse_inject_expression(const char *const s, char **buf,
			struct inject_opts *const fopts,
			const bool fault_tokens_only)
{
	char *saveptr = NULL;
	char *name = NULL;
	char *token;

	*buf = xstrdup(s);
	for (token = strtok_r(*buf, ":", &saveptr); token;
	     token = strtok_r(NULL, ":", &saveptr)) {
		if (!name)
			name = token;
		else if (!parse_inject_token(token, fopts, fault_tokens_only))
			goto parse_error;
	}

	if (name)
		return name;

parse_error:
	free(*buf);
	return *buf = NULL;
}

static void
qualify_read(const char *const str)
{
	qualify_tokens(str, &read_set, string_to_uint, "descriptor");
}

static void
qualify_write(const char *const str)
{
	qualify_tokens(str, &write_set, string_to_uint, "descriptor");
}

static void
qualify_signals(const char *const str)
{
	qualify_tokens(str, &signal_set, sigstr_to_uint, "signal");
}

static void
qualify_trace(const char *const str)
{
	qualify_syscall_tokens(str, trace_set, "system call");
}

static void
qualify_abbrev(const char *const str)
{
	qualify_syscall_tokens(str, abbrev_set, "system call");
}

static void
qualify_verbose(const char *const str)
{
	qualify_syscall_tokens(str, verbose_set, "system call");
}

static void
qualify_raw(const char *const str)
{
	qualify_syscall_tokens(str, raw_set, "system call");
}

static void
qualify_inject_common(const char *const str,
		      const bool fault_tokens_only,
		      const char *const description)
{
	struct inject_opts opts = {
		.first = 1,
		.step = 1,
		.rval = INJECT_OPTS_RVAL_DEFAULT,
		.signo = 0
	};
	char *buf = NULL;
	char *name = parse_inject_expression(str, &buf, &opts, fault_tokens_only);
	if (!name) {
		error_msg_and_die("invalid %s '%s'", description, str);
	}

	/* If neither of retval, error, or signal is specified, then ... */
	if (opts.rval == INJECT_OPTS_RVAL_DEFAULT && !opts.signo) {
		if (fault_tokens_only) {
			/* in fault= syntax the default error code is ENOSYS. */
			opts.rval = -ENOSYS;
		} else {
			/* in inject= syntax this is not allowed. */
			error_msg_and_die("invalid %s '%s'", description, str);
		}
	}

	struct number_set tmp_set[SUPPORTED_PERSONALITIES];
	memset(tmp_set, 0, sizeof(tmp_set));
	qualify_syscall_tokens(name, tmp_set, description);

	free(buf);

	/*
	 * Initialize inject_vec accourding to tmp_set.
	 * Merge tmp_set into inject_set.
	 */
	unsigned int p;
	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		if (!tmp_set[p].nslots && !tmp_set[p].not) {
			continue;
		}

		if (!inject_vec[p]) {
			inject_vec[p] = xcalloc(nsyscall_vec[p],
					       sizeof(*inject_vec[p]));
		}

		unsigned int i;
		for (i = 0; i < nsyscall_vec[p]; ++i) {
			if (is_number_in_set(i, &tmp_set[p])) {
				add_number_to_set(i, &inject_set[p]);
				inject_vec[p][i] = opts;
			}
		}

		free(tmp_set[p].vec);
	}
}

static void
qualify_fault(const char *const str)
{
	qualify_inject_common(str, true, "fault argument");
}

static void
qualify_inject(const char *const str)
{
	qualify_inject_common(str, false, "inject argument");
}

static const struct qual_options {
	const char *name;
	void (*qualify)(const char *);
} qual_options[] = {
	{ "trace",	qualify_trace	},
	{ "t",		qualify_trace	},
	{ "abbrev",	qualify_abbrev	},
	{ "a",		qualify_abbrev	},
	{ "verbose",	qualify_verbose	},
	{ "v",		qualify_verbose	},
	{ "raw",	qualify_raw	},
	{ "x",		qualify_raw	},
	{ "signal",	qualify_signals	},
	{ "signals",	qualify_signals	},
	{ "s",		qualify_signals	},
	{ "read",	qualify_read	},
	{ "reads",	qualify_read	},
	{ "r",		qualify_read	},
	{ "write",	qualify_write	},
	{ "writes",	qualify_write	},
	{ "w",		qualify_write	},
	{ "fault",	qualify_fault	},
	{ "inject",	qualify_inject	},
};

void
qualify(const char *str)
{
	const struct qual_options *opt = qual_options;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(qual_options); ++i) {
		const char *p = qual_options[i].name;
		unsigned int len = strlen(p);

		if (strncmp(str, p, len) || str[len] != '=')
			continue;

		opt = &qual_options[i];
		str += len + 1;
		break;
	}

	opt->qualify(str);
}

unsigned int
qual_flags(const unsigned int scno)
{
	return	(is_number_in_set(scno, &trace_set[current_personality])
		   ? QUAL_TRACE : 0)
		| (is_number_in_set(scno, &abbrev_set[current_personality])
		   ? QUAL_ABBREV : 0)
		| (is_number_in_set(scno, &verbose_set[current_personality])
		   ? QUAL_VERBOSE : 0)
		| (is_number_in_set(scno, &raw_set[current_personality])
		   ? QUAL_RAW : 0)
		| (is_number_in_set(scno, &inject_set[current_personality])
		   ? QUAL_INJECT : 0);
}
