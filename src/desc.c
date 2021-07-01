/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xstring.h"

SYS_FUNC(close)
{
	printfd(tcp, tcp->u_arg[0]);

	return RVAL_DECODED;
}

static int
decode_select(struct tcb *const tcp, const kernel_ulong_t *const args,
	      const print_obj_by_addr_fn print_tv_ts,
	      const sprint_obj_by_addr_fn sprint_tv_ts)
{
	/* Kernel truncates args[0] to int, we do the same. */
	int nfds = (int) args[0];

	/* Kernel rejects negative nfds, so we don't parse it either. */
	if (nfds < 0)
		nfds = 0;

	/* Beware of select(2^31-1, NULL, NULL, NULL) and similar... */
	if (nfds > 1024*1024)
		nfds = 1024*1024;

	/*
	 * We had bugs a-la "while (j < args[0])" and "umoven(args[0])" below.
	 * Instead of args[0], use nfds for fd count, fdsize for array lengths.
	 */
	int fdsize = (((nfds + 7) / 8) + current_wordsize-1) & -current_wordsize;
	fd_set *fds = NULL;

	if (entering(tcp)) {
		PRINT_VAL_D((int) args[0]);

		if (verbose(tcp) && fdsize > 0)
			fds = malloc(fdsize);
		for (unsigned int i = 0; i < 3; ++i) {
			kernel_ulong_t addr = args[i + 1];

			tprint_arg_next();
			if (!fds) {
				printaddr(addr);
				continue;
			}
			if (umoven_or_printaddr(tcp, addr, fdsize, fds))
				continue;
			tprint_bitset_begin();
			bool next = false;
			for (int j = 0;; ++j) {
				j = next_set_bit(fds, j, nfds);
				if (j < 0)
					break;
				if (next)
					tprint_bitset_next();
				else
					next = true;
				printfd(tcp, j);
			}
			tprint_bitset_end();
		}
		free(fds);
		tprint_arg_next();
		print_tv_ts(tcp, args[4]);
	} else {
		static char outstr[1024];
		char *outptr;
#define end_outstr (outstr + sizeof(outstr))
		int ready_fds;

		if (syserror(tcp))
			return 0;

		ready_fds = tcp->u_rval;
		if (ready_fds == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}

		fds = malloc(fdsize);

		outptr = outstr;
		const char *sep = "";
		for (unsigned int i = 0; i < 3 && ready_fds > 0; ++i) {
			int first = 1;
			kernel_ulong_t addr = args[i + 1];

			if (!addr || !fds || umoven(tcp, addr, fdsize, fds) < 0)
				continue;
			for (int j = 0;; ++j) {
				j = next_set_bit(fds, j, nfds);
				if (j < 0)
					break;
				/* +2 chars needed at the end: ']',NUL */
				if (outptr < end_outstr - (sizeof(", except [") + sizeof(int)*3 + 2)) {
					if (first) {
						outptr = xappendstr(outstr,
							outptr,
							"%s%s [%u",
							sep,
							i == 0 ? "in" : i == 1 ? "out" : "except",
							j
						);
						first = 0;
						sep = ", ";
					} else {
						outptr = xappendstr(outstr,
							outptr,
							" %u", j);
					}
				}
				if (--ready_fds == 0)
					break;
			}
			if (outptr != outstr)
				*outptr++ = ']';
		}
		free(fds);
		/* This contains no useful information on SunOS.  */
		if (args[4]) {
			const char *str = sprint_tv_ts(tcp, args[4]);
			if (outptr + sizeof("left ") + strlen(sep) + strlen(str) < end_outstr) {
				outptr = xappendstr(outstr, outptr,
						    "%sleft %s", sep, str);
			}
		}
		*outptr = '\0';
		tcp->auxstr = outstr;
		return RVAL_STR;
#undef end_outstr
	}
	return 0;
}

#if HAVE_ARCH_OLD_SELECT
SYS_FUNC(oldselect)
{
	kernel_ulong_t *args =
		fetch_indirect_syscall_args(tcp, tcp->u_arg[0], 5);

	if (args) {
		return decode_select(tcp, args, print_timeval, sprint_timeval);
	} else {
		if (entering(tcp))
			printaddr(tcp->u_arg[0]);
		return RVAL_DECODED;
	}
}
#endif /* HAVE_ARCH_OLD_SELECT */

#ifdef ALPHA
SYS_FUNC(osf_select)
{
	return decode_select(tcp, tcp->u_arg, print_timeval32, sprint_timeval32);
}
#endif

SYS_FUNC(select)
{
	return decode_select(tcp, tcp->u_arg, print_timeval, sprint_timeval);
}

static int
do_pselect6(struct tcb *const tcp, const print_obj_by_addr_fn print_ts,
	    const sprint_obj_by_addr_fn sprint_ts)
{
	int rc = decode_select(tcp, tcp->u_arg, print_ts, sprint_ts);
	if (entering(tcp)) {
		tprint_arg_next();
		print_kernel_sigset(tcp, tcp->u_arg[5]);
	}

	return rc;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(pselect6_time32)
{
	return do_pselect6(tcp, print_timespec32, sprint_timespec32);
}
#endif

SYS_FUNC(pselect6_time64)
{
	return do_pselect6(tcp, print_timespec64, sprint_timespec64);
}
