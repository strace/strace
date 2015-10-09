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

#include "xlat/fcntlcmds.h"
#include "xlat/fdflags.h"
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
		case F_GETOWN:
			break;
		default:
			return 0;
		}
		return RVAL_DECODED;
	} else {
		switch (tcp->u_arg[1]) {
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
