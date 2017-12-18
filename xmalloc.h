/*
 * This file contains wrapper functions working with memory allocations,
 * they just terminate the program in case of memory allocation failure.
 * These functions can be used by various binaries included in the strace
 * package.
 *
 * Copyright (c) 2001-2017 The strace developers.
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

#ifndef STRACE_XMALLOC_H
#define STRACE_XMALLOC_H

#include <stddef.h>
#include "gcc_compat.h"

#define xcalloc strace_calloc
#define xmalloc strace_malloc

void *xcalloc(size_t nmemb, size_t size)
	ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1, 2));
void *xmalloc(size_t size) ATTRIBUTE_MALLOC ATTRIBUTE_ALLOC_SIZE((1));
void *xreallocarray(void *ptr, size_t nmemb, size_t size)
	ATTRIBUTE_ALLOC_SIZE((2, 3));

/**
 * Utility function for the simplification of managing various dynamic arrays.
 * Knows better how to resize arrays. Dies if there's no enough memory.
 *
 * @param[in]      ptr       Pointer to the array to be resized. If ptr is NULL,
 *                           new array is allocated.
 * @param[in, out] nmemb     Pointer to the current member count. If ptr is
 *                           NULL, it specifies number of members in the newly
 *                           created array. If ptr is NULL and nmemb is 0,
 *                           number of members in the new array is decided by
 *                           the function. Member count is updated by the
 *                           function to the new value.
 * @param[in]      memb_size Size of array member in bytes.
 * @return                   Pointer to the (re)allocated array.
 */
void *xgrowarray(void *ptr, size_t *nmemb, size_t memb_size);

/*
 * Note that the following two functions return NULL when NULL is specified
 * and not when allocation is failed, since, as the "x" prefix implies,
 * the allocation failure leads to program termination, so we may re-purpose
 * this return value and simplify the idiom "str ? xstrdup(str) : NULL".
 */
char *xstrdup(const char *str) ATTRIBUTE_MALLOC;
char *xstrndup(const char *str, size_t n) ATTRIBUTE_MALLOC;

#endif /* !STRACE_XMALLOC_H */
