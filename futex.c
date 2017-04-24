/*
 * Copyright (c) 2002-2003 Roland McGrath  <roland@redhat.com>
 * Copyright (c) 2007-2008 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef FUTEX_PRIVATE_FLAG
# define FUTEX_PRIVATE_FLAG 128
#endif
#ifndef FUTEX_CLOCK_REALTIME
# define FUTEX_CLOCK_REALTIME 256
#endif

#include "xlat/futexops.h"
#include "xlat/futexwakeops.h"
#include "xlat/futexwakecmps.h"

SYS_FUNC(futex)
{
	const kernel_ulong_t uaddr = tcp->u_arg[0];
	const int op = tcp->u_arg[1];
	const int cmd = op & 127;
	const kernel_ulong_t timeout = tcp->u_arg[3];
	const kernel_ulong_t uaddr2 = tcp->u_arg[4];
	const unsigned int val = tcp->u_arg[2];
	const unsigned int val2 = tcp->u_arg[3];
	const unsigned int val3 = tcp->u_arg[5];
	const char *comment;

	printaddr(uaddr);
	tprints(", ");
	printxval(futexops, op, "FUTEX_???");
	switch (cmd) {
	case FUTEX_WAIT:
		tprintf(", %u", val);
		tprints(", ");
		print_timespec(tcp, timeout);
		break;
	case FUTEX_LOCK_PI:
		tprints(", ");
		print_timespec(tcp, timeout);
		break;
	case FUTEX_WAIT_BITSET:
		tprintf(", %u", val);
		tprints(", ");
		print_timespec(tcp, timeout);
		tprintf(", %#x", val3);
		break;
	case FUTEX_WAKE_BITSET:
		tprintf(", %u", val);
		tprintf(", %#x", val3);
		break;
	case FUTEX_REQUEUE:
		tprintf(", %u", val);
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		break;
	case FUTEX_CMP_REQUEUE:
	case FUTEX_CMP_REQUEUE_PI:
		tprintf(", %u", val);
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		tprintf(", %u", val3);
		break;
	case FUTEX_WAKE_OP:
		tprintf(", %u", val);
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		tprints(", ");
		if ((val3 >> 28) & 8)
			tprints("FUTEX_OP_OPARG_SHIFT<<28|");
		comment = printxval(futexwakeops, (val3 >> 28) & 0x7, NULL)
			? NULL : "FUTEX_OP_???";
		tprints("<<28");
		tprints_comment(comment);
		tprintf("|%#x<<12|", (val3 >> 12) & 0xfff);
		comment = printxval(futexwakecmps, (val3 >> 24) & 0xf, NULL)
			? NULL : "FUTEX_OP_CMP_???";
		tprints("<<24");
		tprints_comment(comment);
		tprintf("|%#x", val3 & 0xfff);
		break;
	case FUTEX_WAIT_REQUEUE_PI:
		tprintf(", %u", val);
		tprints(", ");
		print_timespec(tcp, timeout);
		tprints(", ");
		printaddr(uaddr2);
		break;
	case FUTEX_FD:
	case FUTEX_WAKE:
		tprintf(", %u", val);
		break;
	case FUTEX_UNLOCK_PI:
	case FUTEX_TRYLOCK_PI:
		break;
	default:
		tprintf(", %u", val);
		tprints(", ");
		printaddr(timeout);
		tprints(", ");
		printaddr(uaddr2);
		tprintf(", %#x", val3);
		break;
	}

	return RVAL_DECODED;
}
