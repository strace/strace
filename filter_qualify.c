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
#include "nsig.h"
#include "number_set.h"
#include "filter.h"

struct number_set *read_set;
struct number_set *write_set;
struct number_set *signal_set;

static struct number_set *abbrev_set;
static struct number_set *inject_set;
static struct number_set *raw_set;
static struct number_set *trace_set;
static struct number_set *verbose_set;

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

	if ((val = STR_STRIP_PREFIX(token, "when=")) != token) {
		/*
		 *	== 1+1
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
	} else if ((val = STR_STRIP_PREFIX(token, "error=")) != token) {
		if (fopts->data.flags & INJECT_F_RETVAL)
			return false;
		intval = string_to_uint_upto(val, MAX_ERRNO_VALUE);
		if (intval < 0)
			intval = find_errno_by_name(val);
		if (intval < 1)
			return false;
		fopts->data.rval = -intval;
		fopts->data.flags |= INJECT_F_RETVAL;
	} else if (!fault_tokens_only
		   && (val = STR_STRIP_PREFIX(token, "retval=")) != token) {
		if (fopts->data.flags & INJECT_F_RETVAL)
			return false;
		intval = string_to_uint(val);
		if (intval < 0)
			return false;
		fopts->data.rval = intval;
		fopts->data.flags |= INJECT_F_RETVAL;
	} else if (!fault_tokens_only
		   && (val = STR_STRIP_PREFIX(token, "signal=")) != token) {
		if (fopts->data.flags & INJECT_F_SIGNAL)
			return false;
		intval = sigstr_to_uint(val);
		if (intval < 1 || intval > NSIG_BYTES * 8)
			return false;
		fopts->data.signo = intval;
		fopts->data.flags |= INJECT_F_SIGNAL;
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
	if (!read_set)
		read_set = alloc_number_set_array(1);
	qualify_tokens(str, read_set, string_to_uint, "descriptor");
}

static void
qualify_write(const char *const str)
{
	if (!write_set)
		write_set = alloc_number_set_array(1);
	qualify_tokens(str, write_set, string_to_uint, "descriptor");
}

static void
qualify_signals(const char *const str)
{
	if (!signal_set)
		signal_set = alloc_number_set_array(1);
	qualify_tokens(str, signal_set, sigstr_to_uint, "signal");
}

static void
qualify_trace(const char *const str)
{
	if (!trace_set)
		trace_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, trace_set, "system call");
}

static void
qualify_abbrev(const char *const str)
{
	if (!abbrev_set)
		abbrev_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, abbrev_set, "system call");
}

static void
qualify_verbose(const char *const str)
{
	if (!verbose_set)
		verbose_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, verbose_set, "system call");
}

static void
qualify_raw(const char *const str)
{
	if (!raw_set)
		raw_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, raw_set, "system call");
}

static void
qualify_inject_common(const char *const str,
		      const bool fault_tokens_only,
		      const char *const description)
{
	struct inject_opts opts = {
		.first = 1,
		.step = 1
	};
	char *buf = NULL;
	char *name = parse_inject_expression(str, &buf, &opts, fault_tokens_only);
	if (!name) {
		error_msg_and_die("invalid %s '%s'", description, str);
	}

	/* If neither of retval, error, or signal is specified, then ... */
	if (!opts.data.flags) {
		if (fault_tokens_only) {
			/* in fault= syntax the default error code is ENOSYS. */
			opts.data.rval = -ENOSYS;
			opts.data.flags |= INJECT_F_RETVAL;
		} else {
			/* in inject= syntax this is not allowed. */
			error_msg_and_die("invalid %s '%s'", description, str);
		}
	}

	struct number_set *tmp_set =
		alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(name, tmp_set, description);

	free(buf);

	/*
	 * Initialize inject_vec accourding to tmp_set.
	 * Merge tmp_set into inject_set.
	 */
	unsigned int p;
	for (p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
		if (number_set_array_is_empty(tmp_set, p))
			continue;

		if (!inject_set) {
			inject_set =
				alloc_number_set_array(SUPPORTED_PERSONALITIES);
		}
		if (!inject_vec[p]) {
			inject_vec[p] = xcalloc(nsyscall_vec[p],
					       sizeof(*inject_vec[p]));
		}

		unsigned int i;
		for (i = 0; i < nsyscall_vec[p]; ++i) {
			if (is_number_in_set_array(i, tmp_set, p)) {
				add_number_to_set_array(i, inject_set, p);
				inject_vec[p][i] = opts;
			}
		}
	}

	free_number_set_array(tmp_set, SUPPORTED_PERSONALITIES);
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
		const char *name = qual_options[i].name;
		const size_t len = strlen(name);
		const char *val = str_strip_prefix_len(str, name, len);

		if (val == str || *val != '=')
			continue;
		str = val + 1;
		opt = &qual_options[i];
		break;
	}

	opt->qualify(str);
}

unsigned int
qual_flags(const unsigned int scno)
{
	return	(is_number_in_set_array(scno, trace_set, current_personality)
		   ? QUAL_TRACE : 0)
		| (is_number_in_set_array(scno, abbrev_set, current_personality)
		   ? QUAL_ABBREV : 0)
		| (is_number_in_set_array(scno, verbose_set, current_personality)
		   ? QUAL_VERBOSE : 0)
		| (is_number_in_set_array(scno, raw_set, current_personality)
		   ? QUAL_RAW : 0)
		| (is_number_in_set_array(scno, inject_set, current_personality)
		   ? QUAL_INJECT : 0);
}
