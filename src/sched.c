/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <sched.h>
#include "sched_attr.h"

#include "xlat/schedulers.h"
#include "xlat/sched_flags.h"

SYS_FUNC(sched_getscheduler)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
	} else if (!syserror(tcp)) {
		tcp->auxstr = xlookup(schedulers, (kernel_ulong_t) tcp->u_rval);
		return RVAL_STR;
	}
	return 0;
}

SYS_FUNC(sched_setscheduler)
{
	/* pid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* policy */
	printxval(schedulers, tcp->u_arg[1], "SCHED_???");
	tprint_arg_next();

	/* param */
	printnum_int(tcp, tcp->u_arg[2], "%d");

	return RVAL_DECODED;
}

SYS_FUNC(sched_getparam)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();
	} else {
		/* param */
		printnum_int(tcp, tcp->u_arg[1], "%d");
	}
	return 0;
}

SYS_FUNC(sched_setparam)
{
	/* pid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* param */
	printnum_int(tcp, tcp->u_arg[1], "%d");

	return RVAL_DECODED;
}

SYS_FUNC(sched_get_priority_min)
{
	/* policy */
	printxval(schedulers, tcp->u_arg[0], "SCHED_???");

	return RVAL_DECODED;
}

static int
do_sched_rr_get_interval(struct tcb *const tcp,
			 const print_obj_by_addr_fn print_ts)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();
	} else {
		/* tp */
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			print_ts(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(sched_rr_get_interval_time32)
{
	return do_sched_rr_get_interval(tcp, print_timespec32);
}
#endif

SYS_FUNC(sched_rr_get_interval_time64)
{
	return do_sched_rr_get_interval(tcp, print_timespec64);
}

static void
print_sched_attr(struct tcb *const tcp, const kernel_ulong_t addr,
		 unsigned int usize)
{
	struct sched_attr attr = {};
	unsigned int size;
	bool is_set = false;

	if (usize) {
		/* called from sched_getattr */
		size = usize <= sizeof(attr) ? usize : (unsigned) sizeof(attr);
		if (umoven_or_printaddr(tcp, addr, size, &attr))
			return;
		/* the number of bytes written by the kernel */
		size = attr.size;
	} else {
		/* called from sched_setattr */
		is_set = true;

		if (umove_or_printaddr(tcp, addr, &attr.size))
			return;
		usize = attr.size;
		if (!usize)
			usize = SCHED_ATTR_MIN_SIZE;
		size = usize <= sizeof(attr) ? usize : (unsigned) sizeof(attr);
		if (size >= SCHED_ATTR_MIN_SIZE) {
			if (umoven_or_printaddr(tcp, addr, size, &attr))
				return;
		}
	}

	tprint_struct_begin();
	PRINT_FIELD_U(attr, size);

	if (size < SCHED_ATTR_MIN_SIZE)
		goto end;

	if (!is_set || (int)attr.sched_policy < 0 || !(attr.sched_flags & (SCHED_FLAG_KEEP_POLICY | SCHED_FLAG_KEEP_PARAMS))) {
		tprint_struct_next();
		PRINT_FIELD_XVAL(attr, sched_policy, schedulers,
				 "SCHED_???");
	}
	tprint_struct_next();
	PRINT_FIELD_FLAGS(attr, sched_flags, sched_flags, "SCHED_FLAG_???");


	if (!is_set || !(attr.sched_flags & SCHED_FLAG_KEEP_PARAMS)) {
		tprint_struct_next();
		PRINT_FIELD_D(attr, sched_nice);
		tprint_struct_next();
		PRINT_FIELD_U(attr, sched_priority);
		tprint_struct_next();
		PRINT_FIELD_U(attr, sched_runtime);
		tprint_struct_next();
		PRINT_FIELD_U(attr, sched_deadline);
		tprint_struct_next();
		PRINT_FIELD_U(attr, sched_period);
	}

	if (size < SCHED_ATTR_SIZE_VER1)
		goto end;

	tprint_struct_next();
	PRINT_FIELD_U(attr, sched_util_min);
	tprint_struct_next();
	PRINT_FIELD_U(attr, sched_util_max);

end:
	if ((is_set ? usize : attr.size) > size) {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();
}

SYS_FUNC(sched_setattr)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();

		/* attr */
		print_sched_attr(tcp, tcp->u_arg[1], 0);
	} else {
		struct sched_attr attr;

		if (verbose(tcp) && tcp->u_error == E2BIG
		    && umove(tcp, tcp->u_arg[1], &attr.size) == 0) {
			tprint_value_changed();
			tprint_struct_begin();
			PRINT_FIELD_U(attr, size);
			tprint_struct_end();
		}
		tprint_arg_next();

		/* flags */
		PRINT_VAL_U((unsigned int) tcp->u_arg[2]);
	}

	return 0;
}

SYS_FUNC(sched_getattr)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();
	} else {
		const unsigned int size = tcp->u_arg[2];

		/* attr */
		if (size)
			print_sched_attr(tcp, tcp->u_arg[1], size);
		else
			printaddr(tcp->u_arg[1]);
		tprint_arg_next();

		/* size */
#ifdef AARCH64
		/*
		 * Due to a subtle gcc bug that leads to miscompiled aarch64
		 * kernels, the 3rd argument of sched_getattr is not quite 32-bit
		 * as on other architectures.  For more details see
		 * https://lists.strace.io/pipermail/strace-devel/2017-March/006085.html
		 */
		if (syserror(tcp)) {
			tprint_flags_begin();
			print_abnormal_hi(tcp->u_arg[2]);
			PRINT_VAL_U(size);
			tprint_flags_end();
		} else
#endif
		{
			PRINT_VAL_U(size);
		}
		tprint_arg_next();

		/* flags */
		PRINT_VAL_U((unsigned int) tcp->u_arg[3]);
	}

	return 0;
}
