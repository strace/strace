/*
 * Copyright (c) 2021-2024 The strace developers.
 * Copyright (c) 2021 André Almeida <andrealmeid@collabora.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/futex.h>
#include "xlat/futex2_sizes.h"
#include "xlat/futex2_flags.h"

static void
print_futex2_flags(unsigned int flags)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_X(flags);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	tprint_flags_begin();

	printxvals_ex(flags & FUTEX2_SIZE_MASK, NULL, XLAT_STYLE_ABBREV,
		      futex2_sizes, NULL);
	flags &= ~FUTEX2_SIZE_MASK;

	if (flags) {
		tprint_flags_or();
		printflags_ex(flags, NULL, XLAT_STYLE_ABBREV,
			      futex2_flags, NULL);
	}

	tprint_flags_end();

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
                tprint_comment_end();
}

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
	PRINT_FIELD_OBJ_VAL(*waiter, flags, print_futex2_flags);

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

SYS_FUNC(futex_wake)
{
	const kernel_ulong_t uaddr = tcp->u_arg[0];
	const kernel_ulong_t mask = tcp->u_arg[1];
	const int nr = tcp->u_arg[2];
	const unsigned int flags = tcp->u_arg[3];

	/* uaddr */
	printaddr(uaddr);
	tprint_arg_next();

	/* mask */
	printxval64(futexbitset, mask, NULL);
	tprint_arg_next();

	/* nr */
	PRINT_VAL_D(nr);
	tprint_arg_next();

	/* flags */
	print_futex2_flags(flags);

	return RVAL_DECODED;
}

SYS_FUNC(futex_wait)
{
	const kernel_ulong_t uaddr = tcp->u_arg[0];
	const kernel_ulong_t val = tcp->u_arg[1];
	const kernel_ulong_t mask = tcp->u_arg[2];
	const unsigned int flags = tcp->u_arg[3];
	const kernel_ulong_t timeout = tcp->u_arg[4];
	const unsigned int clockid = tcp->u_arg[5];

	/* uaddr */
	printaddr(uaddr);
	tprint_arg_next();

	/* val */
	PRINT_VAL_U(val);
	tprint_arg_next();

	/* mask */
	printxval64(futexbitset, mask, NULL);
	tprint_arg_next();

	/* flags */
	print_futex2_flags(flags);
	tprint_arg_next();

	/* timeout */
	print_timespec64(tcp, timeout);
	tprint_arg_next();

	/* clockid */
	printxval(clocknames, clockid, "CLOCK_???");

	return RVAL_DECODED;
}

SYS_FUNC(futex_requeue)
{
	const kernel_ulong_t waiters = tcp->u_arg[0];
	const unsigned int flags = tcp->u_arg[1];
	const int nr_wake = tcp->u_arg[2];
	const int nr_requeue = tcp->u_arg[3];

	/* waiters */
	print_waiter_array(tcp, waiters, 2);
	tprint_arg_next();

	/* flags */
	PRINT_VAL_X(flags);
	tprint_arg_next();

	/* nr_wake */
	PRINT_VAL_D(nr_wake);
	tprint_arg_next();

	/* nr_requeue */
	PRINT_VAL_D(nr_requeue);

	return RVAL_DECODED;
}
