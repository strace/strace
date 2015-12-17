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

#include "xlat/schedulers.h"
#include "xlat/sched_flags.h"

SYS_FUNC(sched_getscheduler)
{
	if (entering(tcp)) {
		tprintf("%d", (int) tcp->u_arg[0]);
	} else if (!syserror(tcp)) {
		tcp->auxstr = xlookup(schedulers, tcp->u_rval);
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
print_sched_attr(struct tcb *tcp, const long addr, unsigned int size)
{
	struct {
		uint32_t size;
		uint32_t sched_policy;
		uint64_t sched_flags;
		uint32_t sched_nice;
		uint32_t sched_priority;
		uint64_t sched_runtime;
		uint64_t sched_deadline;
		uint64_t sched_period;
	} attr = {};

	if (size > sizeof(attr))
		size = sizeof(attr);
	if (umoven_or_printaddr(tcp, addr, size, &attr))
		return;

	tprintf("{size=%u, sched_policy=", attr.size);
	printxval(schedulers, attr.sched_policy, "SCHED_???");
	tprints(", sched_flags=");
	printflags(sched_flags, attr.sched_flags, "SCHED_FLAG_???");
	tprintf(", sched_nice=%d", attr.sched_nice);
	tprintf(", sched_priority=%u", attr.sched_priority);
	tprintf(", sched_runtime=%" PRIu64, attr.sched_runtime);
	tprintf(", sched_deadline=%" PRIu64, attr.sched_deadline);
	tprintf(", sched_period=%" PRIu64 "}", attr.sched_period);
}

SYS_FUNC(sched_setattr)
{
	tprintf("%d, ", (int) tcp->u_arg[0]);
	print_sched_attr(tcp, tcp->u_arg[1], 0x100);
	tprintf(", %u", (unsigned int) tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(sched_getattr)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		print_sched_attr(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %u, %u",
			(unsigned int) tcp->u_arg[2],
			(unsigned int) tcp->u_arg[3]);
	}

	return 0;
}
