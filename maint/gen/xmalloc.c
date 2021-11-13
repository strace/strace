/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmalloc.h"

static void
die_out_of_memory(void)
{
	fprintf(stderr, "allocation failed\n");
	exit(EXIT_FAILURE);
}

void
free_by_pointer(void *p) {
	void **pp = (void **) p;
	free(*pp);
	*pp = NULL;
}

void *
xmalloc(size_t n)
{
	void *p = malloc(n);

	if (!p)
		die_out_of_memory();

	return p;
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);

	if (!p)
		die_out_of_memory();

	return p;
}

char *
xstrdup(const char *str)
{
	if (!str)
		return NULL;

	char *p = strdup(str);

	if (!p)
		die_out_of_memory();

	return p;
}

char *
xasprintf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	char *res;
	if (vasprintf(&res, fmt, ap) < 0)
		die_out_of_memory();

	va_end(ap);
	return res;
}
