/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xstring.h"
#include "kernel_fcntl.h"
#include "number_set.h"
#include <linux/openat2.h>
#include <linux/fcntl.h>

#include "xlat/open_access_modes.h"
#include "xlat/open_mode_flags.h"
#include "xlat/open_resolve_flags.h"

/* The fd is an "int", so when decoding x86 on x86_64, we need to force sign
 * extension to get the right value.  We do this by declaring fd as int here.
 */
void
print_dirfd(struct tcb *tcp, int fd)
{
	if (fd == AT_FDCWD) {
		print_xlat_d(AT_FDCWD);

		if (!is_number_in_set(DECODE_FD_PATH, decode_fd_set))
			goto done;

		int proc_pid = get_proc_pid(tcp->pid);
		if (!proc_pid)
			goto done;

		static const char cwd_path[] = "/proc/%u/cwd";
		char linkpath[sizeof(cwd_path) + sizeof(int) * 3];
		xsprintf(linkpath, cwd_path, proc_pid);

		char buf[PATH_MAX];
		ssize_t n = readlink(linkpath, buf, sizeof(buf));
		if ((size_t) n >= sizeof(buf))
			goto done;

		tprint_associated_info_begin();
		print_quoted_string_ex(buf, n,
				       QUOTE_OMIT_LEADING_TRAILING_QUOTES,
				       "<>");
		tprint_associated_info_end();
done:		;
	} else {
		printfd(tcp, fd);
	}
#ifdef ENABLE_SECONTEXT
	tcp->last_dirfd = fd;
#endif
}

/*
 * low bits of the open(2) flags define access mode,
 * other bits are real flags.
 */
static const char *
sprint_open_modes64(uint64_t flags)
{
	static char outstr[sizeof("flags O_ACCMODE")];
	char *p;
	char sep;
	const char *str;

	sep = ' ';
	p = stpcpy(outstr, "flags");
	str = xlookup(open_access_modes, flags & 3);
	if (str) {
		*p++ = sep;
		p = stpcpy(p, str);
		flags &= ~3;
		if (!flags)
			return outstr;
		sep = '|';
	}
	*p = '\0';

	return sprintflags_ex(outstr, open_mode_flags, flags, sep,
			      XLAT_STYLE_ABBREV) ?: outstr;
}

const char *
sprint_open_modes(unsigned int flags)
{
	return sprint_open_modes64(flags);
}

static void
tprint_open_modes64(uint64_t flags)
{
	print_xlat_ex(flags, sprint_open_modes64(flags) + sizeof("flags"),
		      XLAT_STYLE_DEFAULT);
}
void
tprint_open_modes(unsigned int flags)
{
	tprint_open_modes64(flags);
}

static int
decode_open(struct tcb *tcp, int offset)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[offset]);
	tprint_arg_next();

	/* flags */
	tprint_open_modes(tcp->u_arg[offset + 1]);

	if (tcp->u_arg[offset + 1] & (O_CREAT | __O_TMPFILE)) {
		tprint_arg_next();

		/* mode */
		print_numeric_umode_t(tcp->u_arg[offset + 2]);
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(open)
{
	return decode_open(tcp, 0);
}

static void
print_open_how(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size)
{
	enum { OPEN_HOW_MIN_SIZE = 24 };

	struct open_how how;

	if (size < OPEN_HOW_MIN_SIZE) {
		printaddr(addr);
		return;
	}
	if (umoven_or_printaddr(tcp, addr, MIN(size, sizeof(how)), &how))
		return;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_VAL(how, flags, tprint_open_modes64);
	if ((how.flags & (O_CREAT | __O_TMPFILE)) || how.mode) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_U(how, mode, print_numeric_ll_umode_t);
	}
	tprint_struct_next();
	PRINT_FIELD_FLAGS(how, resolve, open_resolve_flags, "RESOLVE_???");

	if (size > sizeof(how)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(how),
				    MIN(size, get_pagesize()), QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

SYS_FUNC(openat)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	return decode_open(tcp, 1);
}

SYS_FUNC(openat2)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* how */
	print_open_how(tcp, tcp->u_arg[2], tcp->u_arg[3]);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(tcp->u_arg[3]);

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(creat)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* mode */
	print_numeric_umode_t(tcp->u_arg[1]);

	return RVAL_DECODED | RVAL_FD;
}
