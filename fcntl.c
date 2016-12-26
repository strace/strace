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
#include "flock.h"

#include "xlat/f_owner_types.h"
#include "xlat/f_seals.h"
#include "xlat/fcntlcmds.h"
#include "xlat/fcntl64cmds.h"
#include "xlat/fdflags.h"
#include "xlat/lockfcmds.h"
#include "xlat/notifyflags.h"

static void
print_struct_flock64(const struct_kernel_flock64 *fl, const int getlk)
{
	tprints("{l_type=");
	printxval(lockfcmds, (unsigned short) fl->l_type, "F_???");
	tprints(", l_whence=");
	printxval(whence_codes, (unsigned short) fl->l_whence, "SEEK_???");
	tprintf(", l_start=%" PRId64 ", l_len=%" PRId64,
		(int64_t) fl->l_start, (int64_t) fl->l_len);
	if (getlk)
		tprintf(", l_pid=%lu", (unsigned long) fl->l_pid);
	tprints("}");
}

static void
printflock64(struct tcb *const tcp, const kernel_ulong_t addr, const int getlk)
{
	struct_kernel_flock64 fl;

	if (fetch_struct_flock64(tcp, addr, &fl))
		print_struct_flock64(&fl, getlk);
}

static void
printflock(struct tcb *const tcp, const kernel_ulong_t addr, const int getlk)
{
	struct_kernel_flock64 fl;

	if (fetch_struct_flock(tcp, addr, &fl))
		print_struct_flock64(&fl, getlk);
}

static void
print_f_owner_ex(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct { int type, pid; } owner;

	if (umove_or_printaddr(tcp, addr, &owner))
		return;

	tprints("{type=");
	printxval(f_owner_types, owner.type, "F_OWNER_???");
	tprintf(", pid=%d}", owner.pid);
}

static int
print_fcntl(struct tcb *tcp)
{
	const unsigned int cmd = tcp->u_arg[1];

	switch (cmd) {
	case F_SETFD:
		tprints(", ");
		printflags(fdflags, tcp->u_arg[2], "FD_???");
		break;
	case F_SETOWN:
	case F_SETPIPE_SZ:
		tprintf(", %" PRI_kld, tcp->u_arg[2]);
		break;
	case F_DUPFD:
	case F_DUPFD_CLOEXEC:
		tprintf(", %" PRI_kld, tcp->u_arg[2]);
		return RVAL_DECODED | RVAL_FD;
	case F_SETFL:
		tprints(", ");
		tprint_open_modes(tcp->u_arg[2]);
		break;
	case F_SETLK:
	case F_SETLKW:
		tprints(", ");
		printflock(tcp, tcp->u_arg[2], 0);
		break;
	case F_OFD_SETLK:
	case F_OFD_SETLKW:
		tprints(", ");
		printflock64(tcp, tcp->u_arg[2], 0);
		break;
	case F_SETOWN_EX:
		tprints(", ");
		print_f_owner_ex(tcp, tcp->u_arg[2]);
		break;
	case F_NOTIFY:
		tprints(", ");
		printflags64(notifyflags, tcp->u_arg[2], "DN_???");
		break;
	case F_SETLEASE:
		tprints(", ");
		printxval64(lockfcmds, tcp->u_arg[2], "F_???");
		break;
	case F_ADD_SEALS:
		tprints(", ");
		printflags64(f_seals, tcp->u_arg[2], "F_SEAL_???");
		break;
	case F_SETSIG:
		tprints(", ");
		tprints(signame(tcp->u_arg[2]));
		break;
	case F_GETOWN:
	case F_GETPIPE_SZ:
		break;
	case F_GETFD:
		if (entering(tcp) || syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("flags ", fdflags,
					  (kernel_ulong_t) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GETFL:
		if (entering(tcp) || syserror(tcp))
			return 0;
		tcp->auxstr = sprint_open_modes(tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GETLK:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printflock(tcp, tcp->u_arg[2], 1);
		break;
	case F_OFD_GETLK:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printflock64(tcp, tcp->u_arg[2], 1);
		break;
	case F_GETOWN_EX:
		if (entering(tcp))
			return 0;
		tprints(", ");
		print_f_owner_ex(tcp, tcp->u_arg[2]);
		break;
	case F_GETLEASE:
		if (entering(tcp) || syserror(tcp))
			return 0;
		tcp->auxstr = xlookup(lockfcmds, (kernel_ulong_t) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GET_SEALS:
		if (entering(tcp) || syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("seals ", f_seals,
					  (kernel_ulong_t) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GETSIG:
		if (entering(tcp) || syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = signame(tcp->u_rval);
		return RVAL_STR;
	default:
		tprintf(", %#" PRI_klx, tcp->u_arg[2]);
		break;
	}
	return RVAL_DECODED;
}

SYS_FUNC(fcntl)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		const unsigned int cmd = tcp->u_arg[1];
		const char *str = xlookup(fcntlcmds, cmd);
		if (str) {
			tprints(str);
		} else {
			/*
			 * fcntl syscall does not recognize these
			 * constants, but we would like to show them
			 * for better debugging experience.
			 */
			printxval(fcntl64cmds, cmd, "F_???");
		}
	}
	return print_fcntl(tcp);
}

SYS_FUNC(fcntl64)
{
	const unsigned int cmd = tcp->u_arg[1];
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		const char *str = xlookup(fcntl64cmds, cmd);
		if (str) {
			tprints(str);
		} else {
			printxval(fcntlcmds, cmd, "F_???");
		}
	}
	switch (cmd) {
		case F_SETLK64:
		case F_SETLKW64:
			tprints(", ");
			printflock64(tcp, tcp->u_arg[2], 0);
			return RVAL_DECODED;
		case F_GETLK64:
			if (exiting(tcp)) {
				tprints(", ");
				printflock64(tcp, tcp->u_arg[2], 1);
			}
			return 0;
	}
	return print_fcntl(tcp);
}
