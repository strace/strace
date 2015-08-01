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
#include <fcntl.h>
#include <sys/file.h>

#include "xlat/fcntlcmds.h"
#include "xlat/fdflags.h"
#include "xlat/flockcmds.h"
#include "xlat/lockfcmds.h"
#include "xlat/notifyflags.h"

/*
 * Assume that F_SETLK64, F_SETLKW64, and F_GETLK64 are either defined
 * or not defined altogether.
 */
#if defined(F_SETLK64) && F_SETLK64 + 0 != F_SETLK
# define USE_PRINTFLOCK64 1
#else
# define USE_PRINTFLOCK64 0
#endif

#if USE_PRINTFLOCK64

# ifndef HAVE_STRUCT_FLOCK64
struct flock64 {
	short int l_type, l_whence;
	int64_t l_start, l_len;
	int l_pid;
};
# endif

static void
printflock64(struct tcb *tcp, long addr, int getlk)
{
	struct flock64 fl;

	if (umove_or_printaddr(tcp, addr, &fl))
		return;
	tprints("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprints(", whence=");
	printxval(whence_codes, fl.l_whence, "SEEK_???");
	tprintf(", start=%lld, len=%lld", (long long) fl.l_start, (long long) fl.l_len);
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprints("}");
}
#endif /* USE_PRINTFLOCK64 */

static void
printflock(struct tcb *tcp, long addr, int getlk)
{
	struct flock fl;

#if SUPPORTED_PERSONALITIES > 1
	if (
# if SIZEOF_OFF_T > SIZEOF_LONG
	    current_personality != DEFAULT_PERSONALITY &&
# endif
	    current_wordsize != sizeof(fl.l_start)) {
		if (current_wordsize == 4) {
			/* 32-bit x86 app on x86_64 and similar cases */
			struct {
				short int l_type;
				short int l_whence;
				int32_t l_start; /* off_t */
				int32_t l_len; /* off_t */
				int32_t l_pid; /* pid_t */
			} fl32;
			if (umove_or_printaddr(tcp, addr, &fl32))
				return;
			fl.l_type = fl32.l_type;
			fl.l_whence = fl32.l_whence;
			fl.l_start = fl32.l_start;
			fl.l_len = fl32.l_len;
			fl.l_pid = fl32.l_pid;
		} else {
			/* let people know we have a problem here */
			tprintf("<decode error: unsupported wordsize %d>",
				current_wordsize);
			return;
		}
	} else
#endif
	if (umove_or_printaddr(tcp, addr, &fl))
		return;
	tprints("{type=");
	printxval(lockfcmds, fl.l_type, "F_???");
	tprints(", whence=");
	printxval(whence_codes, fl.l_whence, "SEEK_???");
#if SIZEOF_OFF_T > SIZEOF_LONG
	tprintf(", start=%lld, len=%lld", fl.l_start, fl.l_len);
#else
	tprintf(", start=%ld, len=%ld", fl.l_start, fl.l_len);
#endif
	if (getlk)
		tprintf(", pid=%lu}", (unsigned long) fl.l_pid);
	else
		tprints("}");
}

SYS_FUNC(fcntl)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printxval(fcntlcmds, tcp->u_arg[1], "F_???");
		switch (tcp->u_arg[1]) {
		case F_SETFD:
			tprints(", ");
			printflags(fdflags, tcp->u_arg[2], "FD_???");
			break;
		case F_SETOWN: case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
		case F_DUPFD_CLOEXEC:
#endif
			tprintf(", %ld", tcp->u_arg[2]);
			break;
		case F_SETFL:
			tprints(", ");
			tprint_open_modes(tcp->u_arg[2]);
			break;
		case F_SETLK: case F_SETLKW:
			tprints(", ");
			printflock(tcp, tcp->u_arg[2], 0);
			break;
#if USE_PRINTFLOCK64
		case F_SETLK64: case F_SETLKW64:
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 0);
			break;
#endif /* USE_PRINTFLOCK64 */
#ifdef F_NOTIFY
		case F_NOTIFY:
			tprints(", ");
			printflags(notifyflags, tcp->u_arg[2], "DN_???");
			break;
#endif
#ifdef F_SETLEASE
		case F_SETLEASE:
			tprints(", ");
			printxval(lockfcmds, tcp->u_arg[2], "F_???");
			break;
#endif
		}
	} else {
		switch (tcp->u_arg[1]) {
		case F_DUPFD:
#ifdef F_DUPFD_CLOEXEC
		case F_DUPFD_CLOEXEC:
#endif
		case F_SETFD: case F_SETFL:
		case F_SETLK: case F_SETLKW:
		case F_SETOWN: case F_GETOWN:
#ifdef F_NOTIFY
		case F_NOTIFY:
#endif
#ifdef F_SETLEASE
		case F_SETLEASE:
#endif
			break;
		case F_GETFD:
			if (syserror(tcp) || tcp->u_rval == 0)
				return 0;
			tcp->auxstr = sprintflags("flags ", fdflags, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETFL:
			if (syserror(tcp))
				return 0;
			tcp->auxstr = sprint_open_modes(tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
		case F_GETLK:
			tprints(", ");
			printflock(tcp, tcp->u_arg[2], 1);
			break;
#if USE_PRINTFLOCK64
		case F_GETLK64:
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 1);
			break;
#endif
#ifdef F_GETLEASE
		case F_GETLEASE:
			if (syserror(tcp))
				return 0;
			tcp->auxstr = xlookup(lockfcmds, tcp->u_rval);
			return RVAL_HEX|RVAL_STR;
#endif
		default:
			tprintf(", %#lx", tcp->u_arg[2]);
			break;
		}
	}
	return 0;
}

#ifdef LOCK_SH

SYS_FUNC(flock)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(flockcmds, tcp->u_arg[1], "LOCK_???");

	return RVAL_DECODED;
}
#endif /* LOCK_SH */

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

#if defined(ALPHA)
SYS_FUNC(getdtablesize)
{
	return 0;
}
#endif

static int
decode_select(struct tcb *tcp, long *args, enum bitness_t bitness)
{
	int i, j;
	int nfds, fdsize;
	fd_set *fds = NULL;
	const char *sep;
	long arg;

	/* Kernel truncates arg[0] to int, we do the same. */
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
			arg = args[i+1];
			tprints(", ");
			if (!fds) {
				printaddr(arg);
				continue;
			}
			if (umoven_or_printaddr(tcp, arg, fdsize, fds))
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
		printtv_bitness(tcp, args[4], bitness, 0);
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

			arg = args[i+1];
			if (!arg || !fds || umoven(tcp, arg, fdsize, fds) < 0)
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
			if (outptr < end_outstr - (10 + TIMEVAL_TEXT_BUFSIZE)) {
				outptr += sprintf(outptr, "%sleft ", sep);
				outptr = sprinttv(outptr, tcp, args[4], bitness, /*special:*/ 0);
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
	long long_args[5];
#undef oldselect_args
#if SIZEOF_LONG == 4
# define oldselect_args long_args
#else
	unsigned int oldselect_args[5];
	unsigned int i;
#endif

	if (umove(tcp, tcp->u_arg[0], &oldselect_args) < 0) {
		printaddr(tcp->u_arg[0]);
		return 0;
	}
#ifndef oldselect_args
	for (i = 0; i < 5; i++) {
		long_args[i] = oldselect_args[i];
	}
#endif
	return decode_select(tcp, long_args, BITNESS_CURRENT);
#undef oldselect_args
}

#ifdef ALPHA
SYS_FUNC(osf_select)
{
	return decode_select(tcp, tcp->u_arg, BITNESS_32);
}
#endif

SYS_FUNC(select)
{
	return decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
}

SYS_FUNC(pselect6)
{
	int rc = decode_select(tcp, tcp->u_arg, BITNESS_CURRENT);
	if (entering(tcp)) {
		long r;
		struct {
			unsigned long ptr;
			unsigned long len;
		} data;

		tprints(", ");
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
		if (current_wordsize == 4) {
			struct {
				uint32_t ptr;
				uint32_t len;
			} data32;
			r = umove_or_printaddr(tcp, tcp->u_arg[5], &data32);
			data.ptr = data32.ptr;
			data.len = data32.len;
		} else
#endif
			r = umove_or_printaddr(tcp, tcp->u_arg[5], &data);
		if (r == 0) {
			tprints("{");
			/* NB: kernel requires data.len == NSIG / 8 */
			print_sigset_addr_len(tcp, data.ptr, data.len);
			tprintf(", %lu}", data.len);
		}
	}
	return rc;
}
