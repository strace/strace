/*
 * Copyright (c) 2003-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
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
print_nodemask(struct tcb *tcp, unsigned long addr, unsigned long maxnodes)
{
	const unsigned long nlongs =
		(maxnodes + 8 * current_wordsize - 2) / (8 * current_wordsize);
	const unsigned long size = nlongs * current_wordsize;
	const unsigned long end = addr + size;

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp))
	    || end <= addr || size / current_wordsize != nlongs
	    || nlongs < maxnodes / (8 * current_wordsize)) {
		printaddr(addr);
		return;
	}

	const unsigned long abbrev_end =
		(abbrev(tcp) && max_strlen < nlongs) ?
			addr + max_strlen * current_wordsize : end;
	unsigned long cur;
	for (cur = addr; cur < end; cur += current_wordsize) {
		if (cur != addr)
			tprints(", ");

		unsigned long n;
		if (umove_ulong_or_printaddr(tcp, cur, &n))
			break;

		if (cur == addr)
			tprints("[");

		if (cur >= abbrev_end) {
			tprints("...");
			cur = end;
			break;
		}

		tprintf("%#0*lx", (int) current_wordsize * 2 + 2, n);
	}
	if (cur != addr)
		tprints("]");
}

SYS_FUNC(migrate_pages)
{
	tprintf("%d, %lu, ", (int) tcp->u_arg[0], tcp->u_arg[1]);
	print_nodemask(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	tprints(", ");
	print_nodemask(tcp, tcp->u_arg[3], tcp->u_arg[1]);

	return RVAL_DECODED;
}

#include "xlat/policies.h"
#include "xlat/mbindflags.h"

SYS_FUNC(mbind)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %lu, ", tcp->u_arg[1]);
	printxval(policies, tcp->u_arg[2], "MPOL_???");
	tprints(", ");
	print_nodemask(tcp, tcp->u_arg[3], tcp->u_arg[4]);
	tprintf(", %lu, ", tcp->u_arg[4]);
	printflags(mbindflags, tcp->u_arg[5], "MPOL_???");

	return RVAL_DECODED;
}

SYS_FUNC(set_mempolicy)
{
	printxval(policies, tcp->u_arg[0], "MPOL_???");
	tprints(", ");
	print_nodemask(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %lu", tcp->u_arg[2]);

	return RVAL_DECODED;
}

#include "xlat/mempolicyflags.h"

SYS_FUNC(get_mempolicy)
{
	if (exiting(tcp)) {
		int pol;
		if (!umove_or_printaddr(tcp, tcp->u_arg[0], &pol)) {
			tprints("[");
			printxval(policies, pol, "MPOL_???");
			tprints("]");
		}
		tprints(", ");
		print_nodemask(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu, ", tcp->u_arg[2]);
		printaddr(tcp->u_arg[3]);
		tprints(", ");
		printflags(mempolicyflags, tcp->u_arg[4], "MPOL_???");
	}
	return 0;
}

#include "xlat/move_pages_flags.h"

SYS_FUNC(move_pages)
{
	if (entering(tcp)) {
		unsigned long npages = tcp->u_arg[1];
		tprintf("%ld, %lu, ", tcp->u_arg[0], npages);
		if (tcp->u_arg[2] == 0)
			tprints("NULL, ");
		else {
			unsigned int i;
			long puser = tcp->u_arg[2];
			tprints("{");
			for (i = 0; i < npages; ++i) {
				void *p;
				if (i > 0)
					tprints(", ");
				if (umove(tcp, puser, &p) < 0) {
					tprints("???");
					break;
				}
				tprintf("%p", p);
				puser += sizeof(void *);
			}
			tprints("}, ");
		}
		if (tcp->u_arg[3] == 0)
			tprints("NULL, ");
		else {
			unsigned int i;
			long nodeuser = tcp->u_arg[3];
			tprints("{");
			for (i = 0; i < npages; ++i) {
				int node;
				if (i > 0)
					tprints(", ");
				if (umove(tcp, nodeuser, &node) < 0) {
					tprints("???");
					break;
				}
				tprintf("%#x", node);
				nodeuser += sizeof(int);
			}
			tprints("}, ");
		}
	} else {
		unsigned long npages = tcp->u_arg[1];
		if (tcp->u_arg[4] == 0)
			tprints("NULL, ");
		else {
			unsigned int i;
			long statususer = tcp->u_arg[4];
			tprints("{");
			for (i = 0; i < npages; ++i) {
				int status;
				if (i > 0)
					tprints(", ");
				if (umove(tcp, statususer, &status) < 0) {
					tprints("???");
					break;
				}
				tprintf("%#x", status);
				statususer += sizeof(int);
			}
			tprints("}, ");
		}
		printflags(move_pages_flags, tcp->u_arg[5], "MPOL_???");
	}
	return 0;
}
