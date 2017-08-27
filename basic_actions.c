/*
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
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

bool
is_traced(struct tcb *tcp)
{
	return traced(tcp);
}

bool
not_injected(struct tcb *tcp)
{
	return !inject(tcp);
}

void
apply_trace(struct tcb *tcp, void *priv_data)
{
	tcp->qual_flg |= QUAL_TRACE;
}

void
apply_raw(struct tcb *tcp, void *priv_data)
{
	tcp->qual_flg |= QUAL_RAW;
}

void
apply_abbrev(struct tcb *tcp, void *priv_data)
{
	tcp->qual_flg |= QUAL_ABBREV;
}

void
apply_verbose(struct tcb *tcp, void *priv_data)
{
	tcp->qual_flg |= QUAL_VERBOSE;
}

void
apply_inject(struct tcb *tcp, void *priv_data)
{
	struct inject_opts *opts = priv_data;

	tcp->qual_flg |= QUAL_INJECT;
	if (!tcp->inject_vec[current_personality])
		tcp->inject_vec[current_personality] =
			xcalloc(nsyscalls, sizeof(struct inject_opts));
	if (scno_in_range(tcp->scno)
	    && !tcp->inject_vec[current_personality][tcp->scno].data.flags)
		tcp->inject_vec[current_personality][tcp->scno] = *opts;
}

static void *
parse_inject_common(const char *str, bool fault_tokens_only,
		    const char *description)
{
	struct inject_opts *opts = xmalloc(sizeof(struct inject_opts));
	char *copy = xstrdup(str);

	parse_inject_common_args(copy, opts, fault_tokens_only, false);
	if (!opts->data.flags)
		error_msg_and_die("invalid %s argument '%s'",
				  description, str ? str : "");
	free(copy);
	return opts;
}

void *
parse_inject(const char *str)
{
	return parse_inject_common(str, false, "inject");
}

void
apply_fault(struct tcb *tcp, void *priv_data)
{
	apply_inject(tcp, priv_data);
}

void *
parse_fault(const char *str)
{
	return parse_inject_common(str, true, "fault");
}

void
apply_read(struct tcb *tcp, void *_priv_data)
{
	tcp->qual_flg |= QUAL_READ;
}

void
apply_write(struct tcb *tcp, void *_priv_data)
{
	tcp->qual_flg |= QUAL_WRITE;
}
