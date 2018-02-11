/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
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
#include "delay.h"
#include "retval.h"

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
	if (*s >= '0' && *s <= '9')
		return string_to_uint_upto(s, 255);

	if (strncasecmp(s, "SIG", 3) == 0)
		s += 3;

	for (int i = 0; i <= 255; ++i) {
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
	for (unsigned int i = 1; i < nerrnos; ++i) {
		if (errnoent[i] && (strcasecmp(name, errnoent[i]) == 0))
			return i;
	}

	return -1;
}

static bool
parse_delay_token(const char *input, struct inject_opts *fopts, bool isenter)
{
       unsigned flag = isenter ? INJECT_F_DELAY_ENTER : INJECT_F_DELAY_EXIT;

       if (fopts->data.flags & flag) /* duplicate */
               return false;
       long long intval = string_to_ulonglong(input);
       if (intval < 0) /* couldn't parse */
               return false;

       if (fopts->data.delay_idx == (uint16_t) -1)
               fopts->data.delay_idx = alloc_delay_data();
       /* populate .ts_enter or .ts_exit */
       fill_delay_data(fopts->data.delay_idx, intval, isenter);
       fopts->data.flags |= flag;

       return true;
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
		if (fopts->data.flags & (INJECT_F_ERROR | INJECT_F_RETVAL))
			return false;
		intval = string_to_uint_upto(val, MAX_ERRNO_VALUE);
		if (intval < 0)
			intval = find_errno_by_name(val);
		if (intval < 1)
			return false;
		fopts->data.rval_idx = retval_new(intval);
		fopts->data.flags |= INJECT_F_ERROR;
	} else if (!fault_tokens_only
		   && (val = STR_STRIP_PREFIX(token, "retval=")) != token) {

		if (fopts->data.flags & (INJECT_F_ERROR | INJECT_F_RETVAL))
			return false;

		errno = 0;
		char *endp;
		unsigned long long ullval = strtoull(val, &endp, 0);
		if (endp == val || *endp || (kernel_ulong_t) ullval != ullval
		    || ((ullval == 0 || ullval == ULLONG_MAX) && errno))
			return false;

#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
		bool inadvertent_fault_injection = false;
#endif

#if !HAVE_ARCH_DEDICATED_ERR_REG
		if ((kernel_long_t) ullval < 0
		    && (kernel_long_t) ullval >= -MAX_ERRNO_VALUE) {
# if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
			inadvertent_fault_injection = true;
# endif
			error_msg("Inadvertent injection of error %" PRI_kld
				  " is possible for retval=%llu",
				  -(kernel_long_t) ullval, ullval);
		}
# if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
		else if ((int) ullval < 0 && (int) ullval >= -MAX_ERRNO_VALUE) {
			inadvertent_fault_injection = true;
			error_msg("Inadvertent injection of error %d is"
				  " possible in compat personality for"
				  " retval=%llu",
				  -(int) ullval, ullval);
		}
# endif
#endif

#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
		if (!inadvertent_fault_injection
		    && (unsigned int) ullval != ullval) {
			error_msg("Injected return value %llu will be"
				  " clipped to %u in compat personality",
				  ullval, (unsigned int) ullval);
		}
#endif

		fopts->data.rval_idx = retval_new(ullval);
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
	} else if (!fault_tokens_only
		&& (val = STR_STRIP_PREFIX(token, "delay_enter=")) != token) {
		if (!parse_delay_token(val, fopts, true))
			return false;
	} else if (!fault_tokens_only
		&& (val = STR_STRIP_PREFIX(token, "delay_exit=")) != token) {
		if (!parse_delay_token(val, fopts, false))
			return false;
	} else {
		return false;
	}

	return true;
}

static const char *
parse_inject_expression(char *const str,
			struct inject_opts *const fopts,
			const bool fault_tokens_only)
{
	if (str[0] == '\0' || str[0] == ':')
		return "";

	char *saveptr = NULL;
	const char *name = strtok_r(str, ":", &saveptr);

	char *token;
	while ((token = strtok_r(NULL, ":", &saveptr))) {
		if (!parse_inject_token(token, fopts, fault_tokens_only))
			return NULL;
	}

	return name;
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
	qualify_syscall_tokens(str, trace_set);
}

static void
qualify_abbrev(const char *const str)
{
	if (!abbrev_set)
		abbrev_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, abbrev_set);
}

static void
qualify_verbose(const char *const str)
{
	if (!verbose_set)
		verbose_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, verbose_set);
}

static void
qualify_raw(const char *const str)
{
	if (!raw_set)
		raw_set = alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(str, raw_set);
}

static void
qualify_inject_common(const char *const str,
		      const bool fault_tokens_only,
		      const char *const description)
{
	struct inject_opts opts = {
		.first = 1,
		.step = 1,
		.data = {
			.delay_idx = -1
		}
	};
	char *copy = xstrdup(str);
	const char *name =
		parse_inject_expression(copy, &opts, fault_tokens_only);
	if (!name)
		error_msg_and_die("invalid %s '%s'", description, str);

	struct number_set *tmp_set =
		alloc_number_set_array(SUPPORTED_PERSONALITIES);
	qualify_syscall_tokens(name, tmp_set);

	free(copy);

	/* If neither of retval, error, signal or delay is specified, then ... */
	if (!opts.data.flags) {
		if (fault_tokens_only) {
			/* in fault= syntax the default error code is ENOSYS. */
			opts.data.rval_idx = retval_new(ENOSYS);
			opts.data.flags |= INJECT_F_ERROR;
		} else {
			/* in inject= syntax this is not allowed. */
			error_msg_and_die("invalid %s '%s'", description, str);
		}
	}

	/*
	 * Initialize inject_vec according to tmp_set.
	 * Merge tmp_set into inject_set.
	 */
	for (unsigned int p = 0; p < SUPPORTED_PERSONALITIES; ++p) {
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

		for (unsigned int i = 0; i < nsyscall_vec[p]; ++i) {
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

	for (unsigned int i = 0; i < ARRAY_SIZE(qual_options); ++i) {
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
