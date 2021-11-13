/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef XMALLOC_H
# define XMALLOC_H

# define ATTRIBUTE_FORMAT(args)		__attribute__((__format__ args))
# define ATTRIBUTE_MALLOC		__attribute__((__malloc__))
# define ATTRIBUTE_ALLOC_SIZE(args)	__attribute__((__alloc_size__ args))
# define ATTRIBUTE_CLEANUP(args)	__attribute__((__cleanup__(args)))

# define CLEANUP_FREE ATTRIBUTE_CLEANUP(free_by_pointer)

void
free_by_pointer(void *);

void *
xmalloc(size_t)
	ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1));

void *
xcalloc(size_t nmemb, size_t size)
	ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1, 2));

char *
xstrdup(const char *);

char *xasprintf(const char *fmt, ...)
	ATTRIBUTE_FORMAT((printf, 1, 2)) ATTRIBUTE_MALLOC;

#endif /* XMALLOC_H */
