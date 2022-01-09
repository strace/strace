/*
 * Copyright (c) 2021-2022 The strace developers.
 * Copyright (c) 2021 André Almeida <andrealmeid@collabora.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/futex.h>
#include "xlat/futex_waiter_flags.h"

struct print_waiter_data {
	unsigned int count;
};

static bool
print_waiter(struct tcb * const tcp, void * const elem_buf,
	     const size_t elem_size, void * const data)
{
	struct futex_waitv *waiter = elem_buf;
	struct print_waiter_data *p = data;

	if (p->count++ >= FUTEX_WAITV_MAX) {
		tprint_more_data_follows();
		return false;
	}

	tprint_struct_begin();
	PRINT_FIELD_X(*waiter, val);

	tprint_struct_next();
	PRINT_FIELD_ADDR64(*waiter, uaddr);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(*waiter, flags, futex_waiter_flags, "FUTEX_???");

	if (waiter->__reserved) {
		tprint_struct_next();
		PRINT_FIELD_X(*waiter, __reserved);
	}

	tprint_struct_end();
	return true;
}

static void
print_waiter_array(struct tcb * const tcp, const kernel_ulong_t waiters,
		   const unsigned int nr_futexes)
{
	struct futex_waitv buf;
	struct print_waiter_data data = {};

	print_array(tcp, waiters, nr_futexes, &buf, sizeof(buf),
		    tfetch_mem, print_waiter, &data);
}

SYS_FUNC(futex_waitv)
{
	const kernel_ulong_t waiters = tcp->u_arg[0];
	const unsigned int nr_futexes = tcp->u_arg[1];
	const unsigned int flags = tcp->u_arg[2];
	const kernel_ulong_t timeout = tcp->u_arg[3];
	const unsigned int clockid = tcp->u_arg[4];

	print_waiter_array(tcp, waiters, nr_futexes);
	tprint_arg_next();
	PRINT_VAL_U(nr_futexes);
	tprint_arg_next();
	PRINT_VAL_X(flags);
	tprint_arg_next();
	print_timespec64(tcp, timeout);
	tprint_arg_next();
	printxval(clocknames, clockid, "CLOCK_???");

	return RVAL_DECODED;
}
