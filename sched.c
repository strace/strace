/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include <sched.h>
#include "sched_attr.h"

#include "xlat/schedulers.h"
#include "xlat/sched_flags.h"

SYS_FUNC(sched_getscheduler)
{
	if (entering(tcp)) {
		tprintf("%d", (int) tcp->u_arg[0]);
	} else if (!syserror(tcp)) {
		tcp->auxstr = xlookup(schedulers, (kernel_ulong_t) tcp->u_rval);
		if (tcp->auxstr != NULL)
			return RVAL_STR;
	}
	return 0;
}

SYS_FUNC(sched_setscheduler)
{
	tprintf("%d, ", (int) tcp->u_arg[0]);
	printxval(schedulers, tcp->u_arg[1], "SCHED_???");
	tprints(", ");
	printnum_int(tcp, tcp->u_arg[2], "%d");

	return RVAL_DECODED;
}

SYS_FUNC(sched_getparam)
{
	if (entering(tcp))
		tprintf("%d, ", (int) tcp->u_arg[0]);
	else
		printnum_int(tcp, tcp->u_arg[1], "%d");
	return 0;
}

SYS_FUNC(sched_setparam)
{
	tprintf("%d, ", (int) tcp->u_arg[0]);
	printnum_int(tcp, tcp->u_arg[1], "%d");

	return RVAL_DECODED;
}

SYS_FUNC(sched_get_priority_min)
{
	printxval(schedulers, tcp->u_arg[0], "SCHED_???");

	return RVAL_DECODED;
}

SYS_FUNC(sched_rr_get_interval)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			print_timespec(tcp, tcp->u_arg[1]);
	}
	return 0;
}

static void
print_sched_attr(struct tcb *const tcp, const kernel_ulong_t addr,
		 unsigned int usize)
{
	struct sched_attr attr = {};
	unsigned int size;

	if (usize) {
		/* called from sched_getattr */
		size = usize <= sizeof(attr) ? usize : (unsigned) sizeof(attr);
		if (umoven_or_printaddr(tcp, addr, size, &attr))
			return;
		/* the number of bytes written by the kernel */
		size = attr.size;
	} else {
		/* called from sched_setattr */
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

	tprintf("{size=%u", attr.size);

	if (size >= SCHED_ATTR_MIN_SIZE) {
		tprints(", sched_policy=");
		printxval(schedulers, attr.sched_policy, "SCHED_???");
		tprints(", sched_flags=");
		printflags64(sched_flags, attr.sched_flags, "SCHED_FLAG_???");

#define PRINT_SCHED_FIELD(field, fmt)			\
		tprintf(", " #field "=%" fmt, attr.field)

		PRINT_SCHED_FIELD(sched_nice, "d");
		PRINT_SCHED_FIELD(sched_priority, "u");
		PRINT_SCHED_FIELD(sched_runtime, PRIu64);
		PRINT_SCHED_FIELD(sched_deadline, PRIu64);
		PRINT_SCHED_FIELD(sched_period, PRIu64);

		if (usize > size)
			tprints(", ...");
	}

	tprints("}");
}

SYS_FUNC(sched_setattr)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		print_sched_attr(tcp, tcp->u_arg[1], 0);
	} else {
		struct sched_attr attr;

		if (verbose(tcp) && tcp->u_error == E2BIG
		    && umove(tcp, tcp->u_arg[1], &attr.size) == 0) {
			tprintf(" => {size=%u}", attr.size);
		}

		tprintf(", %u", (unsigned int) tcp->u_arg[2]);
	}

	return 0;
}

SYS_FUNC(sched_getattr)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		const unsigned int size = tcp->u_arg[2];

		if (size)
			print_sched_attr(tcp, tcp->u_arg[1], size);
		else
			printaddr(tcp->u_arg[1]);
		tprints(", ");
#ifdef AARCH64
		/*
		 * Due to a subtle gcc bug that leads to miscompiled aarch64
		 * kernels, the 3rd argument of sched_getattr is not quite 32-bit
		 * as on other architectures.  For more details see
		 * https://sourceforge.net/p/strace/mailman/message/35721703/
		 */
		if (syserror(tcp))
			print_abnormal_hi(tcp->u_arg[2]);
#endif
		tprintf("%u", size);
		tprintf(", %u", (unsigned int) tcp->u_arg[3]);
	}

	return 0;
}
