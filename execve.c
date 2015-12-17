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
printargv(struct tcb *tcp, long addr)
{
	union {
		unsigned int p32;
		unsigned long p64;
		char data[sizeof(long)];
	} cp;
	const char *sep;
	unsigned int n = 0;
	const unsigned wordsize = current_wordsize;

	cp.p64 = 1;
	for (sep = ""; !abbrev(tcp) || n < max_strlen / 2; sep = ", ", ++n) {
		if (umoven_or_printaddr(tcp, addr, wordsize, cp.data))
			return;
		if (wordsize == 4)
			cp.p64 = cp.p32;
		if (cp.p64 == 0)
			break;
		tprints(sep);
		printstr(tcp, cp.p64, -1);
		addr += wordsize;
	}
	if (cp.p64)
		tprintf("%s...", sep);
}

static void
printargc(const char *fmt, struct tcb *tcp, long addr)
{
	int count;
	char *cp = NULL;

	for (count = 0; !umoven(tcp, addr, current_wordsize, &cp) && cp; count++) {
		addr += current_wordsize;
	}
	tprintf(fmt, count, count == 1 ? "" : "s");
}

static void
decode_execve(struct tcb *tcp, const unsigned int index)
{
	printpath(tcp, tcp->u_arg[index + 0]);
	tprints(", ");

	if (!tcp->u_arg[index + 1] || !verbose(tcp))
		printaddr(tcp->u_arg[index + 1]);
	else {
		tprints("[");
		printargv(tcp, tcp->u_arg[index + 1]);
		tprints("]");
	}
	tprints(", ");

	if (!tcp->u_arg[index + 2] || !verbose(tcp))
		printaddr(tcp->u_arg[index + 2]);
	else if (abbrev(tcp))
		printargc("[/* %d var%s */]", tcp, tcp->u_arg[index + 2]);
	else {
		tprints("[");
		printargv(tcp, tcp->u_arg[index + 2]);
		tprints("]");
	}
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
	if (!tcp->u_arg[1] || !verbose(tcp))
		printaddr(tcp->u_arg[1]);
	else {
		tprints("[");
		printargv(tcp, tcp->u_arg[1]);
		tprints("]");
	}

	return RVAL_DECODED;
}
#endif /* SPARC || SPARC64 */
