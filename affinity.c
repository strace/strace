/*
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2009-2015 Dmitry V. Levin <ldv@altlinux.org>
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

static void
print_affinitylist(struct tcb *tcp, const unsigned long addr, const unsigned int len)
{
	unsigned long w;
	const unsigned int size = len * sizeof(w);
	const unsigned long end = addr + size;
	unsigned long cur, abbrev_end;

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr || !len || size / sizeof(w) != len || end < addr) {
		printaddr(addr);
		return;
	}

	if (abbrev(tcp)) {
		abbrev_end = addr + max_strlen *  sizeof(w);
		if (abbrev_end < addr)
			abbrev_end = end;
	} else {
		abbrev_end = end;
	}

	tprints("[");
	for (cur = addr; cur < end; cur += sizeof(w)) {
		if (cur > addr)
			tprints(", ");
		if (cur >= abbrev_end) {
			tprints("...");
			break;
		}
		if (umove_or_printaddr(tcp, cur, &w))
			break;
		tprintf("%lx", w);
	}
	tprints("]");
}

SYS_FUNC(sched_setaffinity)
{
	tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	print_affinitylist(tcp, tcp->u_arg[2], tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(sched_getaffinity)
{
	if (entering(tcp)) {
		tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	} else {
		print_affinitylist(tcp, tcp->u_arg[2], tcp->u_rval);
	}
	return 0;
}
