/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2011-2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2010-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static void
printargv(struct tcb *const tcp, kernel_ulong_t addr)
{
	if (!addr || !verbose(tcp)) {
		printaddr(addr);
		return;
	}

	const unsigned int wordsize = current_wordsize;
	kernel_ulong_t prev_addr = 0;
	unsigned int n = 0;

	for (;; prev_addr = addr, addr += wordsize, ++n) {
		union {
			unsigned int w32;
			kernel_ulong_t wl;
			char data[sizeof(kernel_ulong_t)];
		} cp;

		if (addr < prev_addr || umoven(tcp, addr, wordsize, cp.data)) {
			if (n == 0) {
				printaddr(addr);
				return;
			}
			tprint_array_next();
			tprint_more_data_follows();
			printaddr_comment(addr);
			break;
		}

		const kernel_ulong_t word = (wordsize == sizeof(cp.w32))
					    ? (kernel_ulong_t) cp.w32 : cp.wl;
		if (n == 0)
			tprint_array_begin();
		if (word == 0)
			break;
		if (n != 0)
			tprint_array_next();

		if (abbrev(tcp) && n >= max_strlen) {
			tprint_more_data_follows();
			break;
		}

		printstr(tcp, word);
	}

	tprint_array_end();
}

static void
printargc(struct tcb *const tcp, kernel_ulong_t addr)
{
	printaddr(addr);

	if (!addr || !verbose(tcp))
		return;

	const unsigned int wordsize = current_wordsize;
	kernel_ulong_t prev_addr = 0;
	unsigned int n;

	for (n = 0; addr > prev_addr; prev_addr = addr, addr += wordsize, ++n) {
		kernel_ulong_t word = 0;
		if (umoven(tcp, addr, wordsize, &word)) {
			if (n == 0)
				return;

			addr = 0;
			break;
		}
		if (word == 0)
			break;
	}
	tprintf_comment("%u var%s%s",
			n, n == 1 ? "" : "s",
			addr < prev_addr ? ", unterminated" : "");
}

static void
decode_execve(struct tcb *tcp, const unsigned int index)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[index + 0]);
	tprint_arg_next();

	/* argv */
	printargv(tcp, tcp->u_arg[index + 1]);
	tprint_arg_next();

	/* envp */
	(abbrev(tcp) ? printargc : printargv) (tcp, tcp->u_arg[index + 2]);
}

SYS_FUNC(execve)
{
	decode_execve(tcp, 0);

	return RVAL_DECODED;
}

SYS_FUNC(execveat)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* pathname, argv, envp */
	decode_execve(tcp, 1);
	tprint_arg_next();

	/* flags */
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

#if defined(SPARC) || defined(SPARC64)
SYS_FUNC(execv)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* argv */
	printargv(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}
#endif /* SPARC || SPARC64 */
