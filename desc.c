/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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

SYS_FUNC(close)
{
	printfd(tcp, tcp->u_arg[0]);

	return RVAL_DECODED;
}

SYS_FUNC(dup)
{
	printfd(tcp, tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_FD;
}

static int
do_dup2(struct tcb *tcp, int flags_arg)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printfd(tcp, tcp->u_arg[1]);
	if (flags_arg >= 0) {
		tprints(", ");
		printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(dup2)
{
	return do_dup2(tcp, -1);
}

SYS_FUNC(dup3)
{
	return do_dup2(tcp, 2);
}

static int
decode_select(struct tcb *const tcp, const kernel_ulong_t *const args,
	      void (*const print_tv_ts) (struct tcb *, kernel_ulong_t),
	      const char * (*const sprint_tv_ts) (struct tcb *, kernel_ulong_t))
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
						outptr += sprintf(outptr, "%s%s [%u",
							sep,
							i == 0 ? "in" : i == 1 ? "out" : "except",
							j
						);
						first = 0;
						sep = ", ";
					}
					else {
						outptr += sprintf(outptr, " %u", j);
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
				outptr += sprintf(outptr, "%sleft %s", sep, str);
			}
		}
		*outptr = '\0';
		tcp->auxstr = outstr;
		return RVAL_STR;
#undef end_outstr
	}
	return 0;
}

SYS_FUNC(oldselect)
{
	kernel_ulong_t select_args[5];
	unsigned int oldselect_args[5];

	if (sizeof(*select_args) == sizeof(*oldselect_args)) {
		if (umove_or_printaddr(tcp, tcp->u_arg[0], &select_args)) {
			return 0;
		}
	} else {
		unsigned int i;

		if (umove_or_printaddr(tcp, tcp->u_arg[0], &oldselect_args)) {
			return 0;
		}

		for (i = 0; i < 5; ++i) {
			select_args[i] = oldselect_args[i];
		}
	}

	return decode_select(tcp, select_args, print_timeval, sprint_timeval);
}

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

SYS_FUNC(pselect6)
{
	int rc = decode_select(tcp, tcp->u_arg, print_timespec, sprint_timespec);
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
