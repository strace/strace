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
get_nodes(struct tcb *tcp, unsigned long ptr, unsigned long maxnodes, int err)
{
	unsigned long nlongs, size, end;

	nlongs = (maxnodes + 8 * sizeof(long) - 1) / (8 * sizeof(long));
	size = nlongs * sizeof(long);
	end = ptr + size;
	if (nlongs == 0 || ((err || verbose(tcp)) && (size * 8 == maxnodes)
			    && (end > ptr))) {
		unsigned long n, cur, abbrev_end;
		int failed = 0;

		if (abbrev(tcp)) {
			abbrev_end = ptr + max_strlen * sizeof(long);
			if (abbrev_end < ptr)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints(", {");
		for (cur = ptr; cur < end; cur += sizeof(long)) {
			if (cur > ptr)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(n), &n) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%#0*lx", (int) sizeof(long) * 2 + 2, n);
		}
		tprints("}");
		if (failed) {
			tprints(" ");
			printaddr(ptr);
		}
	} else {
		tprints(" ");
		printaddr(ptr);
	}
	tprintf(", %lu", maxnodes);
}

SYS_FUNC(migrate_pages)
{
	tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	get_nodes(tcp, tcp->u_arg[2], tcp->u_arg[1], 0);
	tprints(", ");
	get_nodes(tcp, tcp->u_arg[3], tcp->u_arg[1], 0);

	return RVAL_DECODED;
}

#include "xlat/policies.h"
#include "xlat/mbindflags.h"

SYS_FUNC(mbind)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %lu, ", tcp->u_arg[1]);
	printxval(policies, tcp->u_arg[2], "MPOL_???");
	get_nodes(tcp, tcp->u_arg[3], tcp->u_arg[4], 0);
	tprints(", ");
	printflags(mbindflags, tcp->u_arg[5], "MPOL_???");

	return RVAL_DECODED;
}

SYS_FUNC(set_mempolicy)
{
	printxval(policies, tcp->u_arg[0], "MPOL_???");
	get_nodes(tcp, tcp->u_arg[1], tcp->u_arg[2], 0);

	return RVAL_DECODED;
}

#include "xlat/mempolicyflags.h"

SYS_FUNC(get_mempolicy)
{
	if (exiting(tcp)) {
		int pol;
		if (!umove_or_printaddr(tcp, tcp->u_arg[0], &pol))
			printxval(policies, pol, "MPOL_???");
		get_nodes(tcp, tcp->u_arg[1], tcp->u_arg[2], syserror(tcp));
		tprints(", ");
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
