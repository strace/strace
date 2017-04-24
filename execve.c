/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2011-2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2010-2015 Dmitry V. Levin <ldv@altlinux.org>
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
printargv(struct tcb *const tcp, kernel_ulong_t addr)
{
	if (!addr || !verbose(tcp)) {
		printaddr(addr);
		return;
	}

	const char *const start_sep = "[";
	const char *sep = start_sep;
	const unsigned int wordsize = current_wordsize;
	unsigned int n;

	for (n = 0; addr; sep = ", ", addr += wordsize, ++n) {
		union {
			unsigned int p32;
			kernel_ulong_t p64;
			char data[sizeof(kernel_ulong_t)];
		} cp;

		if (umoven(tcp, addr, wordsize, cp.data)) {
			if (sep == start_sep)
				printaddr(addr);
			else
				tprints(", ???]");
			return;
		}
		if (!(wordsize < sizeof(cp.p64) ? cp.p32 : cp.p64)) {
			if (sep == start_sep)
				tprints(start_sep);
			break;
		}
		if (abbrev(tcp) && n >= max_strlen) {
			tprintf("%s...", sep);
			break;
		}
		tprints(sep);
		printstr(tcp, wordsize < sizeof(cp.p64) ? cp.p32 : cp.p64);
	}
	tprints("]");
}

static void
printargc(struct tcb *const tcp, kernel_ulong_t addr)
{
	printaddr(addr);

	if (!addr || !verbose(tcp))
		return;

	bool unterminated = false;
	unsigned int count = 0;
	char *cp = NULL;

	for (; addr; addr += current_wordsize, ++count) {
		if (umoven(tcp, addr, current_wordsize, &cp)) {
			if (!count)
				return;

			unterminated = true;
			break;
		}
		if (!cp)
			break;
	}
	tprintf_comment("%u var%s%s",
		count, count == 1 ? "" : "s",
		unterminated ? ", unterminated" : "");
}

static void
decode_execve(struct tcb *tcp, const unsigned int index)
{
	printpath(tcp, tcp->u_arg[index + 0]);
	tprints(", ");

	printargv(tcp, tcp->u_arg[index + 1]);
	tprints(", ");

	(abbrev(tcp) ? printargc : printargv) (tcp, tcp->u_arg[index + 2]);
}

SYS_FUNC(execve)
{
	decode_execve(tcp, 0);

	return RVAL_DECODED;
}

SYS_FUNC(execveat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	decode_execve(tcp, 1);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

#if defined(SPARC) || defined(SPARC64)
SYS_FUNC(execv)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printargv(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}
#endif /* SPARC || SPARC64 */
