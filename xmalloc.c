/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "error_prints.h"
#include "xmalloc.h"

static void
die_out_of_memory(void)
{
	static int recursed;

	if (recursed)
		exit(1);
	recursed = 1;

	error_msg_and_die("Out of memory");
}

void *
xmalloc(size_t size)
{
	void *p = malloc(size);

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

#define HALF_SIZE_T	(((size_t) 1) << (sizeof(size_t) * 4))

void *
xallocarray(size_t nmemb, size_t size)
{
	size_t bytes = nmemb * size;

	if ((nmemb | size) >= HALF_SIZE_T &&
	    size && bytes / size != nmemb)
		die_out_of_memory();

	void *p = malloc(bytes);

	if (!p)
		die_out_of_memory();

	return p;
}

void *
xreallocarray(void *ptr, size_t nmemb, size_t size)
{
	size_t bytes = nmemb * size;

	if ((nmemb | size) >= HALF_SIZE_T &&
	    size && bytes / size != nmemb)
		die_out_of_memory();

	void *p = realloc(ptr, bytes);

	if (!p)
		die_out_of_memory();

	return p;
}

void *
xgrowarray(void *const ptr, size_t *const nmemb, const size_t memb_size)
{
	/* this is the same value as glibc DEFAULT_MXFAST */
	enum { DEFAULT_ALLOC_SIZE = 64 * SIZEOF_LONG / 4 };

	size_t grow_memb;

	if (ptr == NULL)
		grow_memb = *nmemb ? 0 :
			(DEFAULT_ALLOC_SIZE + memb_size - 1) / memb_size;
	else
		grow_memb = (*nmemb >> 1) + 1;

	if ((*nmemb + grow_memb) < *nmemb)
		die_out_of_memory();

	*nmemb += grow_memb;

	return xreallocarray(ptr, *nmemb, memb_size);
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
xstrndup(const char *str, size_t n)
{
	char *p;

	if (!str)
		return NULL;

#ifdef HAVE_STRNDUP
	p = strndup(str, n);
#else
	p = xmalloc(n + 1);
#endif

	if (!p)
		die_out_of_memory();

#ifndef HAVE_STRNDUP
	strncpy(p, str, n);
	p[n] = '\0';
#endif

	return p;
}
