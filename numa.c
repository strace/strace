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

static bool
print_node(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	if (elem_size < sizeof(kernel_ulong_t)) {
		tprintf("%#0*x", (int) elem_size * 2 + 2,
			* (unsigned int *) elem_buf);
	} else {
		tprintf("%#0*" PRI_klx, (int) elem_size * 2 + 2,
			* (kernel_ulong_t *) elem_buf);
	}

	return true;
}

static void
print_nodemask(struct tcb *const tcp, const kernel_ulong_t addr,
	       const kernel_ulong_t maxnodes)
{
	const unsigned int bits_per_long = 8 * current_wordsize;
	const kernel_ulong_t nmemb =
		(maxnodes + bits_per_long - 2) / bits_per_long;

	if (nmemb < maxnodes / bits_per_long ||
	    (maxnodes && !nmemb)) {
		printaddr(addr);
		return;
	}

	kernel_ulong_t buf;
	print_array(tcp, addr, nmemb, &buf, current_wordsize,
		    umoven_or_printaddr, print_node, 0);
}

SYS_FUNC(migrate_pages)
{
	tprintf("%d, %" PRI_klu ", ", (int) tcp->u_arg[0], tcp->u_arg[1]);
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
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	printxval64(policies, tcp->u_arg[2], "MPOL_???");
	tprints(", ");
	print_nodemask(tcp, tcp->u_arg[3], tcp->u_arg[4]);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[4]);
	printflags(mbindflags, tcp->u_arg[5], "MPOL_???");

	return RVAL_DECODED;
}

SYS_FUNC(set_mempolicy)
{
	printxval(policies, tcp->u_arg[0], "MPOL_???");
	tprints(", ");
	print_nodemask(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprintf(", %" PRI_klu, tcp->u_arg[2]);

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
		tprintf(", %" PRI_klu ", ", tcp->u_arg[2]);
		printaddr(tcp->u_arg[3]);
		tprints(", ");
		printflags64(mempolicyflags, tcp->u_arg[4], "MPOL_???");
	}
	return 0;
}

#include "xlat/move_pages_flags.h"

static bool
print_addr(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	kernel_ulong_t addr;

	if (elem_size < sizeof(addr)) {
		addr = * (unsigned int *) elem_buf;
	} else {
		addr = * (kernel_ulong_t *) elem_buf;
	}

	printaddr(addr);

	return true;
}

static bool
print_status(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const int status = * (int *) elem_buf;

	if (status < 0 && (unsigned) -status < nerrnos)
		tprintf("-%s", errnoent[-status]);
	else
		tprintf("%d", status);

	return true;
}

static bool
print_int(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	tprintf("%d", * (int *) elem_buf);

	return true;
}

SYS_FUNC(move_pages)
{
	const kernel_ulong_t npages = tcp->u_arg[1];
	kernel_ulong_t buf;

	if (entering(tcp)) {
		tprintf("%d, %" PRI_klu ", ", (int) tcp->u_arg[0], npages);
		print_array(tcp, tcp->u_arg[2], npages, &buf, current_wordsize,
			    umoven_or_printaddr, print_addr, 0);
		tprints(", ");
		print_array(tcp, tcp->u_arg[3], npages, &buf, sizeof(int),
			    umoven_or_printaddr, print_int, 0);
		tprints(", ");
	} else {
		print_array(tcp, tcp->u_arg[4], npages, &buf, sizeof(int),
			    umoven_or_printaddr, print_status, 0);
		tprints(", ");
		printflags(move_pages_flags, tcp->u_arg[5], "MPOL_???");
	}
	return 0;
}
