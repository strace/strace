/*
 * Make a hexdump copy of C string
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const char *
hexdump_memdup(const char *src, size_t len)
{
	size_t dst_size = 3 * len + 2;
	assert(dst_size > len);

	char *dst = malloc(dst_size);
	if (!dst)
		perror_msg_and_fail("malloc(%zu)", dst_size);

	char *p = dst;
	const unsigned char *usrc = (const unsigned char *) src;
	for (size_t i = 0; i < len; ++i) {
		unsigned int c = usrc[i];
		*(p++) = ' ';
		if (i == 8)
			*(p++) = ' ';
		*(p++) = "0123456789abcdef"[c >> 4];
		*(p++) = "0123456789abcdef"[c & 0xf];
	}
	*p = '\0';

	return dst;
}

const char *
hexdump_strdup(const char *src)
{
	return hexdump_memdup(src, strlen(src));
}
