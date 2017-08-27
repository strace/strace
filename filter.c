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

#define FILTER_TYPE(name)						\
{#name, parse_ ## name ## _filter, run_ ## name ## _filter,		\
	free_ ## name ## _filter}
/* End of FILTER_TYPE definition. */

static const struct filter_type {
	const char *name;
	void *(*parse_filter)(const char *);
	bool (*run_filter)(struct tcb *, void *);
	void (*free_priv_data)(void *);
} filter_types[] = {
	FILTER_TYPE(syscall),
	FILTER_TYPE(fd),
};
#undef FILTER_TYPE

struct filter {
	const struct filter_type *type;
	void *priv_data;
};

static const struct filter_type *
lookup_filter_type(const char *str)
{
	for (unsigned int i = 0; i < ARRAY_SIZE(filter_types); i++) {
		if (!strcmp(filter_types[i].name, str))
			return &filter_types[i];
	}
	return NULL;
}

struct filter *
add_filter_to_array(struct filter **filters, unsigned int *nfilters,
		    const char *name)
{
	const struct filter_type *type = lookup_filter_type(name);
	struct filter *filter;

	if (!type)
		error_msg_and_die("invalid filter '%s'", name);
	*filters = xreallocarray(*filters, ++(*nfilters),
				 sizeof(struct filter));
	filter = &((*filters)[*nfilters - 1]);
	filter->type = type;
	return filter;
}

void
parse_filter(struct filter *filter, const char *str)
{
	filter->priv_data = filter->type->parse_filter(str);
}

static bool
run_filter(struct tcb *tcp, struct filter *filter)
{
	return filter->type->run_filter(tcp, filter->priv_data);
}

void
run_filters(struct tcb *tcp, struct filter *filters, unsigned int nfilters,
	    bool *variables_buf)
{
	for (unsigned int i = 0; i < nfilters; ++i)
		variables_buf[i] = run_filter(tcp, &filters[i]);
}

void
free_filter(struct filter *filter)
{
	if (!filter)
		return;
	filter->type->free_priv_data(filter->priv_data);
}

void
set_filters_qualify_mode(struct filter **filters, unsigned int *nfilters)
{
	for (unsigned int i = 0; i < *nfilters - 1; ++i)
		free_filter(*filters + i);
	(*filters)[0] = (*filters)[*nfilters - 1];
	*filters = xreallocarray(*filters, 1, sizeof(struct filter));
	*nfilters = 1;
}
