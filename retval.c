/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
