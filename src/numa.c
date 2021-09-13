/*
 * Copyright (c) 2003-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static bool
print_node(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	if (elem_size < sizeof(kernel_ulong_t)) {
		tprintf("%#0*x", (int) elem_size * 2 + 2,
			*(unsigned int *) elem_buf);
	} else {
		tprintf("%#0*" PRI_klx, (int) elem_size * 2 + 2,
			*(kernel_ulong_t *) elem_buf);
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
		    tfetch_mem, print_node, 0);
}

SYS_FUNC(migrate_pages)
{
	/* pid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* maxnode */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* old_nodes */
	print_nodemask(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	tprint_arg_next();

	/* new_nodes */
	print_nodemask(tcp, tcp->u_arg[3], tcp->u_arg[1]);

	return RVAL_DECODED;
}

#include "xlat/mpol_modes.h"
#include "xlat/mpol_mode_flags.h"
#include "xlat/mbind_flags.h"

static void
print_mode(struct tcb *const tcp, const kernel_ulong_t mode_arg)
{
	const kernel_ulong_t flags_mask = MPOL_F_STATIC_NODES |
					  MPOL_F_RELATIVE_NODES |
					  MPOL_F_NUMA_BALANCING;
	const kernel_ulong_t mode = mode_arg & ~flags_mask;
	const unsigned int flags = mode_arg & flags_mask;

	if (!flags) {
		printxval64(mpol_modes, mode, "MPOL_???");
		return;
	}

	const char *mode_str = xlookup(mpol_modes, mode);
	if (!mode_str) {
		printflags64(mpol_mode_flags, mode_arg, "MPOL_???");
		return;
	}

	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_X(mode_arg);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	tprints(mode_str);
	tprints("|");
	printflags_ex(flags, NULL, XLAT_STYLE_ABBREV, mpol_mode_flags, NULL);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
}

SYS_FUNC(mbind)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* len */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* mode */
	print_mode(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* nodemask */
	print_nodemask(tcp, tcp->u_arg[3], tcp->u_arg[4]);
	tprint_arg_next();

	/* maxnode */
	PRINT_VAL_U(tcp->u_arg[4]);
	tprint_arg_next();

	/* flags */
	printflags(mbind_flags, tcp->u_arg[5], "MPOL_???");

	return RVAL_DECODED;
}

SYS_FUNC(set_mempolicy)
{
	/* mode */
	print_mode(tcp, (unsigned int) tcp->u_arg[0]);
	tprint_arg_next();

	/* nodemask */
	print_nodemask(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* maxnode */
	PRINT_VAL_U(tcp->u_arg[2]);

	return RVAL_DECODED;
}

#include "xlat/get_mempolicy_flags.h"

SYS_FUNC(get_mempolicy)
{
	if (exiting(tcp)) {
		/* mode */
		int pol;
		if (!umove_or_printaddr(tcp, tcp->u_arg[0], &pol)) {
			tprint_indirect_begin();
			printxval(mpol_modes, pol, "MPOL_???");
			tprint_indirect_end();
		}
		tprint_arg_next();

		/* nodemask */
		print_nodemask(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprint_arg_next();

		/* maxnode */
		PRINT_VAL_U(tcp->u_arg[2]);
		tprint_arg_next();

		/* addr */
		printaddr(tcp->u_arg[3]);
		tprint_arg_next();

		/* flags */
		printflags64(get_mempolicy_flags, tcp->u_arg[4], "MPOL_???");
	}
	return 0;
}

#include "xlat/move_pages_flags.h"

static bool
print_addr(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	kernel_ulong_t addr;

	if (elem_size < sizeof(addr)) {
		addr = *(unsigned int *) elem_buf;
	} else {
		addr = *(kernel_ulong_t *) elem_buf;
	}

	printaddr(addr);

	return true;
}

static bool
print_status(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const int status = *(int *) elem_buf;

	print_err(status, true);

	return true;
}

SYS_FUNC(move_pages)
{
	const kernel_ulong_t npages = tcp->u_arg[1];
	kernel_ulong_t buf;

	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();

		/* count */
		PRINT_VAL_U(npages);
		tprint_arg_next();

		/* pages */
		print_array(tcp, tcp->u_arg[2], npages, &buf, current_wordsize,
			    tfetch_mem, print_addr, 0);
		tprint_arg_next();

		/* nodes */
		print_array(tcp, tcp->u_arg[3], npages, &buf, sizeof(int),
			    tfetch_mem, print_int_array_member, 0);
		tprint_arg_next();
	} else {
		/* status */
		print_array(tcp, tcp->u_arg[4], npages, &buf, sizeof(int),
			    tfetch_mem, print_status, 0);
		tprint_arg_next();

		/* flags */
		printflags(move_pages_flags, tcp->u_arg[5], "MPOL_???");
	}
	return 0;
}
