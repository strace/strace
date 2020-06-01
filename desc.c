/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2020 The strace developers.
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
	int i, j;
	int nfds, fdsize;
	fd_set *fds = NULL;
	const char *sep;
	kernel_ulong_t addr;

	/* Kernel truncates args[0] to int, we do the same. */
	nfds = (int) args[0];

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
	fdsize = (((nfds + 7) / 8) + current_wordsize-1) & -current_wordsize;

	if (entering(tcp)) {
		tprintf("%d", (int) args[0]);

		if (verbose(tcp) && fdsize > 0)
			fds = malloc(fdsize);
		for (i = 0; i < 3; i++) {
			addr = args[i+1];
			tprints(", ");
			if (!fds) {
				printaddr(addr);
				continue;
			}
			if (umoven_or_printaddr(tcp, addr, fdsize, fds))
				continue;
			tprints("[");
			for (j = 0, sep = "";; j++) {
				j = next_set_bit(fds, j, nfds);
				if (j < 0)
					break;
				tprints(sep);
				printfd(tcp, j);
				sep = " ";
			}
			tprints("]");
		}
		free(fds);
		tprints(", ");
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
		sep = "";
		for (i = 0; i < 3 && ready_fds > 0; i++) {
			int first = 1;

			addr = args[i+1];
			if (!addr || !fds || umoven(tcp, addr, fdsize, fds) < 0)
				continue;
			for (j = 0;; j++) {
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
umove_kulong_array_or_printaddr(struct tcb *const tcp, const kernel_ulong_t addr,
				kernel_ulong_t *const ptr, const size_t n)
{
#ifndef current_klongsize
	if (current_klongsize < sizeof(*ptr)) {
		uint32_t ptr32[n];
		int r = umove_or_printaddr(tcp, addr, &ptr32);
		if (!r) {
			size_t i;

			for (i = 0; i < n; ++i)
				ptr[i] = ptr32[i];
		}
		return r;
	}
#endif /* !current_klongsize */
	return umoven_or_printaddr(tcp, addr, n * sizeof(*ptr), ptr);
}

static int
do_pselect6(struct tcb *const tcp, const print_obj_by_addr_fn print_ts,
	    const sprint_obj_by_addr_fn sprint_ts)
{
	int rc = decode_select(tcp, tcp->u_arg, print_ts, sprint_ts);
	if (entering(tcp)) {
		kernel_ulong_t data[2];

		tprints(", ");
		if (!umove_kulong_array_or_printaddr(tcp, tcp->u_arg[5],
						     data, ARRAY_SIZE(data))) {
			tprints("{");
			/* NB: kernel requires data[1] == NSIG_BYTES */
			print_sigset_addr_len(tcp, data[0], data[1]);
			tprintf(", %" PRI_klu "}", data[1]);
		}
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
