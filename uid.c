/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef STRACE_UID_SIZE
# if STRACE_UID_SIZE != 16
#  error invalid STRACE_UID_SIZE
# endif

# define SIZEIFY(x)		SIZEIFY_(x,STRACE_UID_SIZE)
# define SIZEIFY_(x,size)	SIZEIFY__(x,size)
# define SIZEIFY__(x,size)	x ## size

# define printuid	SIZEIFY(printuid)
# define sys_chown	SIZEIFY(sys_chown)
# define sys_fchown	SIZEIFY(sys_fchown)
# define sys_getgroups	SIZEIFY(sys_getgroups)
# define sys_getresuid	SIZEIFY(sys_getresuid)
# define sys_getuid	SIZEIFY(sys_getuid)
# define sys_setfsuid	SIZEIFY(sys_setfsuid)
# define sys_setgroups	SIZEIFY(sys_setgroups)
# define sys_setresuid	SIZEIFY(sys_setresuid)
# define sys_setreuid	SIZEIFY(sys_setreuid)
# define sys_setuid	SIZEIFY(sys_setuid)
#endif /* STRACE_UID_SIZE */

#include "defs.h"

#ifdef STRACE_UID_SIZE
# if !NEED_UID16_PARSERS
#  undef STRACE_UID_SIZE
# endif
#else
# define STRACE_UID_SIZE 32
#endif

#ifdef STRACE_UID_SIZE

# undef uid_t
# define uid_t		uid_t_(STRACE_UID_SIZE)
# define uid_t_(size)	uid_t__(size)
# define uid_t__(size)	uint ## size ## _t

SYS_FUNC(getuid)
{
	return RVAL_UDECIMAL | RVAL_DECODED;
}

SYS_FUNC(setfsuid)
{
	if (entering(tcp))
		tprintf("%u", (uid_t) tcp->u_arg[0]);

	return RVAL_UDECIMAL | RVAL_DECODED;
}

SYS_FUNC(setuid)
{
	printuid("", tcp->u_arg[0]);

	return RVAL_DECODED;
}

static void
get_print_uid(struct tcb *tcp, const char *prefix, const long addr)
{
	uid_t uid;

	tprints(prefix);
	if (!umove_or_printaddr(tcp, addr, &uid))
		tprintf("[%u]", uid);
}

SYS_FUNC(getresuid)
{
	if (entering(tcp))
		return 0;

	get_print_uid(tcp, "", tcp->u_arg[0]);
	get_print_uid(tcp, ", ", tcp->u_arg[1]);
	get_print_uid(tcp, ", ", tcp->u_arg[2]);

	return 0;
}

SYS_FUNC(setreuid)
{
	printuid("", tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(setresuid)
{
	printuid("", tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(chown)
{
	printpath(tcp, tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(fchown)
{
	printfd(tcp, tcp->u_arg[0]);
	printuid(", ", tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);

	return RVAL_DECODED;
}

void
printuid(const char *text, const unsigned int uid)
{
	if ((unsigned int) -1 == uid || (uid_t) -1 == uid)
		tprintf("%s-1", text);
	else
		tprintf("%s%u", text, uid);
}

SYS_FUNC(setgroups)
{
	unsigned long cur, abbrev_end;
	uid_t gid;
	int failed = 0;
	const unsigned long len = tcp->u_arg[0];
	const unsigned long start = tcp->u_arg[1];
	const unsigned long size = len * sizeof(gid);
	const unsigned long end = start + size;

	tprintf("%lu, ", len);
	if (len == 0) {
		tprints("[]");
		return RVAL_DECODED;
	}
	if (!start || !verbose(tcp) ||
	    size / sizeof(gid) != len || end < start) {
		printaddr(start);
		return RVAL_DECODED;
	}
	if (abbrev(tcp)) {
		abbrev_end = start + max_strlen * sizeof(gid);
		if (abbrev_end < start)
			abbrev_end = end;
	} else {
		abbrev_end = end;
	}
	tprints("[");
	for (cur = start; cur < end; cur += sizeof(gid)) {
		if (cur > start)
			tprints(", ");
		if (cur >= abbrev_end) {
			tprints("...");
			break;
		}
		if (umoven(tcp, cur, sizeof(gid), &gid) < 0) {
			tprints("?");
			failed = 1;
			break;
		}
		tprintf("%u", (unsigned int) gid);
	}
	tprints("]");
	if (failed) {
		tprints(" ");
		printaddr(start);
	}

	return RVAL_DECODED;
}

SYS_FUNC(getgroups)
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		unsigned long cur, abbrev_end;
		uid_t gid;
		int failed = 0;
		const unsigned long len = tcp->u_rval;
		const unsigned long size = len * sizeof(gid);
		const unsigned long start = tcp->u_arg[1];
		const unsigned long end = start + size;

		if (!start) {
			printaddr(start);
			return 0;
		}
		if (len == 0) {
			tprints("[]");
			return 0;
		}
		if (!verbose(tcp) || syserror(tcp) ||
		    size / sizeof(gid) != len || end < start) {
			printaddr(start);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(gid);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(gid)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof(gid), &gid) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			tprintf("%u", (unsigned int) gid);
		}
		tprints("]");
		if (failed) {
			tprints(" ");
			printaddr(start);
		}
	}
	return 0;
}

#endif /* STRACE_UID_SIZE */
