/*
 * Copyright (c) 2018 The strace developers.
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
#include "retval.h"

static kernel_long_t *retval_vec;
static size_t retval_vec_capacity; /* size of the arena */
static size_t retval_vec_size;     /* size of the used arena */

static void
expand_retval_vec(void)
{
	const size_t old_capacity = retval_vec_capacity;
	retval_vec = xgrowarray(retval_vec, &retval_vec_capacity,
				sizeof(*retval_vec));
	memset(retval_vec + old_capacity, 0,
	       (retval_vec_capacity - old_capacity)
	       * sizeof(*retval_vec));
}

uint16_t
retval_new(const kernel_long_t rval)
{
	const uint16_t idx = retval_vec_size;

	if (idx < retval_vec_size)
		error_func_msg_and_die("retval index overflow");

	if (retval_vec_size == retval_vec_capacity)
		expand_retval_vec();

	retval_vec[idx] = rval;
	++retval_vec_size;

	return idx;
}

kernel_long_t
retval_get(const uint16_t rval_idx)
{
	if (rval_idx >= retval_vec_size)
		error_func_msg_and_die("rval_idx >= retval_vec_size");

	return retval_vec[rval_idx];
}
