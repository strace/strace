/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

void die_out_of_memory(void)
{
	static bool recursed = false;

	if (recursed)
		exit(1);
	recursed = 1;

	error_msg_and_die("Out of memory");
}

void *xmalloc(size_t size)
{
	void *p = malloc(size);

	if (!p)
		die_out_of_memory();

	return p;
}

void *xcalloc(size_t nmemb, size_t size)
{
	void *p = calloc(nmemb, size);

	if (!p)
		die_out_of_memory();

	return p;
}

#define HALF_SIZE_T	(((size_t) 1) << (sizeof(size_t) * 4))

void *xreallocarray(void *ptr, size_t nmemb, size_t size)
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

char *xstrdup(const char *str)
{
	char *p = strdup(str);

	if (!p)
		die_out_of_memory();

	return p;
}
